/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <adc.h>
#include <nrfx_adc.h>
#include <errno.h>

#define SYS_LOG_DOMAIN "ADC nRF51"
#define SYS_LOG_LEVEL SYS_LOG_ADC_LEVEL
#include <logging/sys_log.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Typical time to convert single sample in microseconds. */
#define NRF51_TYP_CONV_TIME_10_BIT (68)
#define NRF51_TYP_CONV_TIME_9_BIT  (36)
#define NRF51_TYP_CONV_TIME_8_BIT  (20)

struct adc_nrf51_data_s {
	struct k_sem access_sem;
	struct k_sem done;
	u16_t sample_cnt; /* Counter for samples in single sequence entry. */
	u16_t seq_entry_cnt; /* Counter for sequence entries
			      * in sequence table.
			      */
	nrf_adc_value_t *sample_dst;
};

static struct adc_nrf51_data adc_nrf51_data;

static void adc_nrf51_enable(struct device *dev)
{
	SYS_LOG_INFO("No need to enable ADC");
}

static void adc_nrf51_disable(struct device *dev)
{
	SYS_LOG_INFO("No need to disable ADC");
}

static u32_t adc_nrf51_check_channel_ids(const struct adc_seq_table *seq_table)
{
#if defined(CONFIG_ADC_NRF51_VDD_IN_2_3_PRS) || \
					defined(CONFIG_ADC_NRF51_VDD_IN_1_3_PRS)
	u8_t max_id = 0;
#else
	u8_t max_id = 7;
#endif

	for (u32_t i = 1; i <= seq_table->num_entries; i++) {
		if (seq_table->entries[i - 1].channel_id > max_id) {
			return i;
		}
	}

	return 0;
}

static u32_t adc_nrf51_check_sampling_delays(
					const struct adc_seq_table *seq_table)
{
#if defined(CONFIG_ADC_NRF51_10_BIT_RES)
	u8_t min_int = NRF51_TYP_CONV_TIME_10_BIT;
#elif defined(CONFIG_ADC_NRF51_9_BIT_RES)
	u8_t min_int = NRF51_TYP_CONV_TIME_9_BIT;
#else
	u8_t min_int = NRF51_TYP_CONV_TIME_8_BIT;
#endif

	s32_t delay;

	for (u32_t i = 1; i <= seq_table->num_entries; i++) {
		/* Convert clock ticks delay to microseconds delay. */
		delay = seq_table->entries[i - 1].sampling_delay /
			sys_clock_ticks_per_sec * (1000000);
		if (delay < min_int) {
			return i;
		}
	}

	return 0;
}

static ALWAYS_INLINE u8_t adc_nrf51_resolution(void)
{
#if defined(CONFIG_ADC_NRF51_8_BIT_RES)
	return NRF_ADC_CONFIG_RES_8BIT;
#elif defined(CONFIG_ADC_NRF51_9_BIT_RES)
	return NRF_ADC_CONFIG_RES_9BIT;
#else
	return NRF_ADC_CONFIG_RES_10BIT;
#endif
}

static ALWAYS_INLINE u8_t adc_nrf51_input(void)
{
#if defined(CONFIG_ADC_NRF51_ANG_IN_NO_PRS)
	return NRF_ADC_CONFIG_SCALING_INPUT_FULL_SCALE;
#elif defined(CONFIG_ADC_NRF51_ANG_IN_2_3_PRS)
	return NRF_ADC_CONFIG_SCALING_INPUT_TWO_THIRDS;
#elif defined(CONFIG_ADC_NRF51_ANG_IN_1_3_PRS)
	return NRF_ADC_CONFIG_SCALING_INPUT_ONE_THIRD;
#elif defined(CONFIG_ADC_NRF51_VDD_IN_2_3_PRS)
	return NRF_ADC_CONFIG_SCALING_SUPPLY_TWO_THIRDS;
#else
	return NRF_ADC_CONFIG_SCALING_SUPPLY_ONE_THIRD;
#endif
}

static ALWAYS_INLINE u8_t adc_nrf51_reference(void)
{
#if defined(CONFIG_ADC_NRF51_VBG_REF)
	return NRF_ADC_CONFIG_REF_VBG;
#elif defined(CONFIG_ADC_NRF51_VDD_1_2_REF)
	return NRF_ADC_CONFIG_REF_SUPPLY_ONE_HALF;
#elif defined(CONFIG_ADC_NRF51_VDD_1_3_REF)
	return NRF_ADC_CONFIG_REF_SUPPLY_ONE_THIRD;
#else
	#if defined(CONFIG_ADC_NRF51_ANG_REF_0)
	return NRF_ADC_CONFIG_REF_EXT_REF0;
	#else
	return NRF_ADC_CONFIG_REF_EXT_REF1;
	#endif
#endif
}

static inline u8_t adc_nrf51_map_id_to_input(u8_t channel_id)
{
	switch (channel_id) {
	case 0:
		return NRF_ADC_CONFIG_INPUT_0;
	case 1:
		return NRF_ADC_CONFIG_INPUT_1;
	case 2:
		return NRF_ADC_CONFIG_INPUT_2;
	case 3:
		return NRF_ADC_CONFIG_INPUT_3;
	case 4:
		return NRF_ADC_CONFIG_INPUT_4;
	case 5:
		return NRF_ADC_CONFIG_INPUT_5;
	case 6:
		return NRF_ADC_CONFIG_INPUT_6;
	case 7:
		return NRF_ADC_CONFIG_INPUT_7;
	}
}

static inline void adc_nrf51_set_nrfx_adc_channel(nrfx_adc_channel_t *channel,
						  u8_t channel_id)
{
	channel->config.config.resolution = adc_nrf51_resolution();
	channel->config.config.input = adc_nrf51_input();
	channel->config.config.reference = adc_nrf51_reference();
	channel->config.config.ain = adc_nrf51_map_id_to_input(channel_id);
	/* External reference source is selected together with reference.
	 * channel->config.config.external_reference = ;
	 */
}

static u32_t adc_nrf51_single_sample(const struct adc_seq_entry * const entry)
{
	nrfx_adc_channel_t channel;

	adc_nrf51_data.sample_dst = (nrf_adc_value_t *)(entry->buffer +
			sizeof(nrf_adc_value_t) * adc_nrf51_data.sample_cnt);

	adc_nrf51_set_nrfx_adc_channel(&channel, entry->channel_id);
	nrfx_adc_sample_convert(&channel, NULL);

	k_sem_take(&adc_nrf51_data.done, K_FOREVER);
}


static u32_t adc_nrf51_sample_seq_entry(const struct adc_seq_entry *
								const entry)
{
#if defined(CONFIG_ADC_NRF51_10_BIT_RES)
	u8_t sampling_time = NRF51_TYP_CONV_TIME_10_BIT;
#elif defined(CONFIG_ADC_NRF51_9_BIT_RES)
	u8_t sampling_time = NRF51_TYP_CONV_TIME_9_BIT;
#else
	u8_t sampling_time = NRF51_TYP_CONV_TIME_8_BIT;
#endif

	s32_t delay;

	while (adc_nrf51_data.sample_cnt <
	       (entry->buffer_length / sizeof(nrf_adc_value_t) - 1)) {
		adc_nrf51_single_sample(entry);
		adc_nrf51_data.sample_cnt++;

		if (entry->sampling_delay) {
			delay = entry->.sampling_delay /
				sys_clock_ticks_per_sec * (1000000);
			k_busy_wait(delay - sampling_time);
		}
	}

	adc_nrf51_single_sample(entry);

	adc_nrf51_data->sample_cnt = 0;
}

static u32_t adc_nrf51_sample(const struct adc_seq_table *seq_table)
{
	while (data->seq_entry_cnt < seq_table->num_entries) {
		adc_nrf51_sample_seq_entry();
		data->seq_entry_cnt++;
	}

	adc_nrf51_data->seq_entry_cnt = 0;
}

static int adc_nrf51_read(struct device *dev, struct adc_seq_table *seq_table)
{
	/* If ADC peripheral is busy return an error immediately.
	 * Let the user decide what to do.
	 */
	if (k_sem_take(&adc_nrf51_data->access_sem, K_NO_WAIT)) {
		return -EBUSY;
	} else if (adc_nrf51_check_channel_ids(seq_table)) {
		return -ENXIO;
	} else if (adc_nrf51_check_sampling_delays(seq_table)) {
		return -EINVAL;
	}

	adc_nrf51_sample(seq_table);

	k_sem_give(&data->access_sem);

	return 0;
}

static void adc_nrf51_evt_handler(nrfx_adc_evt_t const *p_event)
{
	switch (p_event->type) {
	case NRFX_ADC_EVT_DONE:
		SYS_LOG_DBG("Unsupported event");
		break;
	case NRFX_ADC_EVT_SAMPLE:
		*adc_nrf51_data.sample_dst = p_event->data.sample;
		k_sem_give(&adc_nrf51_data.done);
		break;
	}
}

static int adc_nrf51_init(struct device *dev)
{
	nrfx_adc_config_t nrfx_adc_cfg;
	ret_code_t ret_code;

	IRQ_CONNECT(ADC_IRQn, CONFIG_ADC_0_IRQ_PRI, adc_nrf51_isr, NULL, 0);

	/* ADC IRQ priority is set via IRQ_CONNECT() */
	nrfx_adc_cfg.interrupt_priority = 0;

	ret_code_t = nrfx_adc_init(&nrfx_adc_cfg, adc_nrf51_evt_handler);

	if (ret_code != NRFX_SUCCESS) {
		SYS_LOG_DBG("Error %d", ret_code);
	}

	return 0;
}

static void adc_nrf51_isr(void *null)
{
	nrfx_adc_irq_handler();
}

DEVICE_DECLARE(adc_nrf51);

static struct adc_nrf51_data adc_nrf51_data = {
	.access_sem = _K_SEM_INITIALIZER(adc_nrf51_data.access_sem, 1, 1),
	.done = _K_SEM_INITIALIZER(adc_nrf51_data.access_sem, 0, 1),
	.sample_cnt = 0,
	.seq_entry_cnt = 0,
};

DEVICE_AND_API_INIT(adc_nrf51, CONFIG_ADC_0_NAME, adc_nrf51_init,
		    &adc_nrf51_data, &adc_nrf5_cfg,
		    APPLICATION, CONFIG_ADC_INIT_PRIORITY,
		    &adc_nrf51_drv_api);

#ifdef __cplusplus
}
#endif
