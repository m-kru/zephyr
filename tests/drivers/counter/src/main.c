/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>

#include <device.h>
#include <counter.h>
#include <misc/printk.h>

#define ALARM (32768)
#define ALARM_2 (5*32768)

#if defined(CONFIG_BOARD_NRF51_PCA10028)
#define COUNTER_NAME CONFIG_RTC_0_NAME
#elif defined(CONFIG_BOARD_NRF52_PCA10040)
#define COUNTER_NAME CONFIG_RTC_1_NAME
#elif defined(CONFIG_BOARD_NRF52840_PCA10056)
#define COUNTER_NAME CONFIG_RTC_2_NAME
#endif

static u32_t some_user_data;

static void counter_cb_1(struct device *counter_dev, void *user_data)
{
	(*((u32_t *)user_data))++;
	printk("Alarm 1, user data = %d\n", (*(u32_t *)user_data));
	counter_set_alarm(counter_dev,
			  counter_cb_1,
			  ALARM,
			  user_data);
}

static void counter_cb_2(struct device *counter_dev, void *user_data)
{
	printk("Alarm 2\n");
	counter_set_alarm(counter_dev, counter_cb_2, ALARM_2, NULL);
}

void main(void)
{
	int err;
	struct device *counter_dev;

	printk("Counter driver test\n");

	counter_dev = device_get_binding(COUNTER_NAME);

	err = counter_start(counter_dev);
	if (!err) {
		printk("Counter started\n");
	} else {
		printk("Error %d while starting counter\n", err);
	}

	err = counter_set_alarm(counter_dev,
				counter_cb_1,
				ALARM,
				(void *)&some_user_data);
	if (err == 0) {
		printk("Alarm set\n");
	} else {
		printk("Error %d while setting the alarm\n", err);
	}

	err = counter_set_alarm(counter_dev,
				counter_cb_2,
				ALARM_2,
				NULL);
	if (err == 0) {
		printk("Alarm set\n");
	} else {
		printk("Error %d while setting the alarm\n", err);
	}

	while (1) {
		/* do nothing */
	}
}
