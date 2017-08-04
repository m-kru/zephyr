#include <errno.h>
#include <counter.h>
#include <soc.h>
#include <nrfx_rtc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTC_COUNTER_BITS (24)
#define RTC_MIN_DELTA (2)

typedef void (*irq_config_fn_t)(void);

/** Configuration data */
struct rtc_nrf5_config {
	nrfx_rtc_t rtc;
	nrfx_rtc_config_t config;
	irq_config_fn_t config_func;
	nrfx_rtc_handler_t evt_handler;
};

struct rtc_nrf5_alarm_info {
	bool busy; /** Information if Capture/Compare channels is allocated. */
	u32_t val; /** Absolute alarm value. */
	counter_callback_t cb_fn; /** Alarm callback function. */
	void *user_data; /** Pointer to alarm callback user data. */
};

/** Driver data */
struct rtc_nrf5_data {
	struct k_sem access_sem;
	struct rtc_nrf5_alarm_info *alarm_info; /* Pointer to array with alarms
						 * informations.
						 */
	volatile u8_t upper_counter; /* Upper 8 bits of 32 bit counter.
				      * Nordic RTC peripherals
				      * counter is only 24 bit long.
				      */
	u8_t cc_alloc; /* Number of times all of the Capture/Compare
			* channels were allocated.
			*/
	volatile u8_t cc_free; /* Number of times all of the Capture/Compare
				* channels were freed.
				*/
	bool enabled;
};

#define DEV_RTC_CFG(dev) \
	((const struct rtc_nrf5_config * const)(dev)->config->config_info)
#define DEV_RTC_DATA(dev) \
	((struct rtc_nrf5_data * const)(dev)->driver_data)

static int rtc_nrf5_start(struct device *dev)
{
	const struct rtc_nrf5_config * const cfg = DEV_RTC_CFG(dev);
	struct rtc_nrf5_data * const data = DEV_RTC_DATA(dev);

	k_sem_take(&data->access_sem, K_FOREVER);

	nrfx_rtc_enable(&cfg->rtc);
	data->enabled = true;

	k_sem_give(&data->access_sem);

	return 0;
}

static int rtc_nrf5_stop(struct device *dev)
{
	const struct rtc_nrf5_config * const cfg = DEV_RTC_CFG(dev);
	struct rtc_nrf5_data * const data = DEV_RTC_DATA(dev);

	k_sem_take(&data->access_sem, K_FOREVER);

	nrfx_rtc_disable(&cfg->rtc);
	data->enabled = false;

	k_sem_give(&data->access_sem);

	return 0;
}

static inline void cancel_alarm(struct device *dev,
				 u8_t cc_index)
{
	const struct rtc_nrf5_config * const cfg = DEV_RTC_CFG(dev);
	struct rtc_nrf5_data * const data = DEV_RTC_DATA(dev);

	nrfx_rtc_cc_disable(&cfg->rtc, cc_index);
	data->alarm_info[cc_index].busy = false;
	data->cc_alloc--;
}

static inline int set_alarm(struct device *dev,
			    counter_callback_t callback,
			    u32_t count, void *user_data)
{
	const struct rtc_nrf5_config *cfg = DEV_RTC_CFG(dev);
	struct rtc_nrf5_data *data = DEV_RTC_DATA(dev);
	u8_t i;

	for (i = 0; i < cfg->rtc.cc_channel_count; i++) {
		if (data->alarm_info[i].busy == false) {
			break;
		}
	}

	data->cc_alloc++;
	data->alarm_info[i].busy = true;

	data->alarm_info[i].val = counter_read(dev) + count;
	data->alarm_info[i].cb_fn = callback;
	data->alarm_info[i].user_data = user_data;

	nrfx_rtc_cc_set(&cfg->rtc, i, data->alarm_info[i].val, true);

	/* Check if min delta condition is still met. If there was any
	 * preemption and condition is not met cancel the alarm.
	 */
	if ((s32_t)(data->alarm_info[i].val - counter_read(dev)) <
	    RTC_MIN_DELTA) {
		cancel_alarm(dev, i);
		return -ECANCELED;
	}

	return 0;
}

static int rtc_nrf5_set_alarm(struct device *dev,
			      counter_callback_t callback,
			      u32_t count, void *user_data)
{
	const struct rtc_nrf5_config * const cfg = DEV_RTC_CFG(dev);
	struct rtc_nrf5_data * const data = DEV_RTC_DATA(dev);
	int ret = 0;

	k_sem_take(&data->access_sem, K_FOREVER);

	if (!data->enabled) {
		ret = -ENOTSUP;
	} else if (data->cc_alloc - data->cc_free >=
		   cfg->rtc.cc_channel_count) {
		ret = -ENOMEM;
	} else if ((s32_t)count <= RTC_MIN_DELTA) {
		ret = -EINVAL;
	} else {
		ret = set_alarm(dev, callback, count, user_data);
	}

	k_sem_give(&data->access_sem);

	return ret;
}

static inline u32_t counter_32_bit(struct device *dev)
{
	const struct rtc_nrf5_config * const cfg = DEV_RTC_CFG(dev);
	struct rtc_nrf5_data * const data = DEV_RTC_DATA(dev);

	return (data->upper_counter << RTC_COUNTER_BITS) +
		nrfx_rtc_counter_get(&cfg->rtc);
}

static u32_t rtc_nrf5_read(struct device *dev)
{
	const struct rtc_nrf5_config * const cfg = DEV_RTC_CFG(dev);

	u32_t rtc_val;

	/* Normally, the overflow will be handled in overflow_evt_handler(),
	 * but this function may be called in a higher priority IRQ context,
	 * and in such case data->upper_counter can be out of sync with
	 * the actual RTC counter value.
	 */
	do {
		rtc_val = counter_32_bit(dev);

		if (nrf_rtc_event_pending(cfg->rtc.p_reg,
					  NRF_RTC_EVENT_OVERFLOW)) {
			/* If overflow event is pending read the 32 bit counter
			 * again (because overflow could happen between previous
			 * rtc_val evaluation and checking if overflow is
			 * pending) and increment it by maximum hardware
			 * counter capacity.
			 */
			rtc_val = counter_32_bit(dev);
			rtc_val += (1 << RTC_COUNTER_BITS);
			break;
		}
		/* Check if 32 bit counter value is still valid. There might be
		 * long preemption between counter value evaluation and quitting
		 * the loop. This mechanism is safer than irq locking because it
		 * does not introduce delay to other time crucial tasks. What is
		 * more irq_lock() can be intentionally disabled with proper
		 * config.
		 */
	} while (rtc_val != counter_32_bit(dev));

	return rtc_val;
}

static u32_t rtc_nrf5_get_pending_int(struct device *dev)
{
	const struct rtc_nrf5_config * const cfg = DEV_RTC_CFG(dev);

	return NVIC_GetPendingIRQ(cfg->rtc.irq);
}

static int rtc_nrf5_init(struct device *dev)
{
	const struct rtc_nrf5_config * const cfg = DEV_RTC_CFG(dev);
	int err;

	cfg->config_func();

	err = nrfx_rtc_init(&cfg->rtc,
			    &cfg->config,
			    cfg->evt_handler);

	/* Only overflow interrupt needs to be enabled for conforming
	 * 32 bit long counter.
	 */
	nrfx_rtc_overflow_enable(&cfg->rtc, true);

	return 0;
}

static inline void overflow_evt_handler(struct device *dev)
{
	struct rtc_nrf5_data * const data = DEV_RTC_DATA(dev);

	data->upper_counter += 1;
}

static inline void cc_evt_handler(struct device *dev, u32_t cc_index)
{
	const struct rtc_nrf5_config * const cfg = DEV_RTC_CFG(dev);
	struct rtc_nrf5_data * const data = DEV_RTC_DATA(dev);

	/* The following checks if rtc counter value is greater than the alarm.
	 * It also handles 32 bit counter wraparound, the cost of it is that
	 * an alarm can not be set when alarm value is greater than the
	 * current RTC counter value + UINT32_MAX/2.
	 */
	if ((s32_t)(data->alarm_info[cc_index].val - rtc_nrf5_read(dev)) <= 0) {
		if (data->alarm_info[cc_index].cb_fn != NULL) {
			data->alarm_info[cc_index].cb_fn(dev,
					data->alarm_info[cc_index].user_data);
		}
		data->alarm_info[cc_index].busy = false;
		data->cc_free++;
	} else {
		nrfx_rtc_cc_set(&cfg->rtc,
				cc_index,
				data->alarm_info[cc_index].val,
				true);
	}
}

static void rtc_nrf5_event_handler(struct device *dev,
				   nrfx_rtc_int_type_t evt)
{
	switch (evt) {
	case NRFX_RTC_INT_COMPARE0:
		cc_evt_handler(dev, 0);
		break;
	case NRFX_RTC_INT_COMPARE1:
		cc_evt_handler(dev, 1);
		break;
	case NRFX_RTC_INT_COMPARE2:
		cc_evt_handler(dev, 2);
		break;
	case NRFX_RTC_INT_COMPARE3:
		cc_evt_handler(dev, 3);
		break;
	case NRFX_RTC_INT_TICK:
		break;
	case NRFX_RTC_INT_OVERFLOW:
		overflow_evt_handler(dev);
		break;
	default:
		break;
	}
}

static const struct counter_driver_api rtc_nrf5_drv_api = {
	.start = rtc_nrf5_start,
	.stop = rtc_nrf5_stop,
	.read = rtc_nrf5_read,
	.set_alarm = rtc_nrf5_set_alarm,
	.get_pending_int = rtc_nrf5_get_pending_int
};

static void rtc_nrf5_isr(void *arg)
{
	((nrfx_irq_handler_t)arg)();
}

#ifdef CONFIG_RTC_0
DEVICE_DECLARE(rtc_nrf5_0);

static void rtc_evt_handler_0(nrfx_rtc_int_type_t int_type)
{
	rtc_nrf5_event_handler(DEVICE_GET(rtc_nrf5_0), int_type);
}

static void cfg_fn_rtc_0(void)
{
	IRQ_CONNECT(RTC0_IRQn, CONFIG_RTC_0_IRQ_PRI,
		    rtc_nrf5_isr, nrfx_rtc_0_irq_handler, 0);
}


static const struct rtc_nrf5_config rtc_nrf5_config_0 = {
	.rtc = NRFX_RTC_INSTANCE(0),
	.config = {
		.prescaler = CONFIG_RTC_0_PRESCALER
		},
	.config_func = cfg_fn_rtc_0,
	.evt_handler = rtc_evt_handler_0
};

static struct rtc_nrf5_alarm_info alarms_info_0[NRF_RTC_CC_CHANNEL_COUNT(0)];

static struct rtc_nrf5_data rtc_nrf5_data_0 = {
	.access_sem = _K_SEM_INITIALIZER(rtc_nrf5_data_0.access_sem, 1, 1),
	.alarm_info = alarms_info_0,
};

DEVICE_AND_API_INIT(rtc_nrf5_0, CONFIG_RTC_0_NAME, rtc_nrf5_init,
		    &rtc_nrf5_data_0, &rtc_nrf5_config_0,
		    PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &rtc_nrf5_drv_api);

#endif /* CONFIG_RTC_0 */

#ifdef CONFIG_RTC_1
DEVICE_DECLARE(rtc_nrf5_1);

static void rtc_evt_handler_1(nrfx_rtc_int_type_t int_type)
{
	rtc_nrf5_event_handler(DEVICE_GET(rtc_nrf5_1), int_type);
}

static void cfg_fn_rtc_1(void)
{
	IRQ_CONNECT(RTC1_IRQn, CONFIG_RTC_1_IRQ_PRI,
		    rtc_nrf5_isr, nrfx_rtc_1_irq_handler, 0);
}


static const struct rtc_nrf5_config rtc_nrf5_config_1 = {
	.rtc = NRFX_RTC_INSTANCE(1),
	.config = {
		.prescaler = CONFIG_RTC_1_PRESCALER
		},
	.config_func = cfg_fn_rtc_1,
	.evt_handler = rtc_evt_handler_1
};

static struct rtc_nrf5_alarm_info alarms_info_1[NRF_RTC_CC_CHANNEL_COUNT(1)];

static struct rtc_nrf5_data rtc_nrf5_data_1 = {
	.access_sem = _K_SEM_INITIALIZER(rtc_nrf5_data_1.access_sem, 1, 1),
	.alarm_info = alarms_info_1,
};

DEVICE_AND_API_INIT(rtc_nrf5_1, CONFIG_RTC_1_NAME, rtc_nrf5_init,
		    &rtc_nrf5_data_1, &rtc_nrf5_config_1,
		    PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &rtc_nrf5_drv_api);

#endif /* CONFIG_RTC_1 */

#ifdef CONFIG_RTC_2
#	if defined(NRF51)
		#error "NRF51 family does not have RTC_2 peripheral."
#	elif defined(NRF52832_XXAA) || defined(NRF52840_XXAA)
DEVICE_DECLARE(rtc_nrf5_2);

static void rtc_evt_handler_2(nrfx_rtc_int_type_t int_type)
{
	rtc_nrf5_event_handler(DEVICE_GET(rtc_nrf5_2), int_type);
}

static void cfg_fn_rtc_2(void)
{
	IRQ_CONNECT(RTC2_IRQn, CONFIG_RTC_2_IRQ_PRI,
		    rtc_nrf5_isr, nrfx_rtc_2_irq_handler, 0);
}


static const struct rtc_nrf5_config rtc_nrf5_config_2 = {
	.rtc = NRFX_RTC_INSTANCE(2),
	.config = {
		.prescaler = CONFIG_RTC_2_PRESCALER
		},
	.config_func = cfg_fn_rtc_2,
	.evt_handler = rtc_evt_handler_2
};

static struct rtc_nrf5_alarm_info alarms_info_2[NRF_RTC_CC_CHANNEL_COUNT(2)];

static struct rtc_nrf5_data rtc_nrf5_data_2 = {
	.access_sem = _K_SEM_INITIALIZER(rtc_nrf5_data_2.access_sem, 1, 1),
	.alarm_info = alarms_info_2,
};

DEVICE_AND_API_INIT(rtc_nrf5_2, CONFIG_RTC_2_NAME, rtc_nrf5_init,
		    &rtc_nrf5_data_2, &rtc_nrf5_config_2,
		    PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    &rtc_nrf5_drv_api);
#	endif
#endif /* CONFIG_RTC_2 */

#ifdef __cplusplus
}
#endif
