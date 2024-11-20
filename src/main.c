/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   500

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);


void reset_to_uf2(void) {
  NRF_POWER->GPREGRET = 0x57; // 0xA8 OTA, 0x4e Serial
  NVIC_SystemReset();         // or sd_nvic_SystemReset();
}

int main(void)
{
	int ret;
	bool led_state = true;

	printf("Start of Program V 2.05 \n");


	// if (!gpio_is_ready_dt(&led)) {
	// 	return 0;
	// }
	// if (!gpio_is_ready_dt(&led1)) {
	// 	return 0;
	// }
	// if (!gpio_is_ready_dt(&led2)) {
	// 	return 0;
	// }

	// ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	// ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	// ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	

	if (ret < 0) {
		return 0;
	}

	printf("Initializaion complete\n");

	int counter=0;

	while (1) {
		// ret = gpio_pin_toggle_dt(&led);
		// ret = gpio_pin_toggle_dt(&led1);
		// ret = gpio_pin_toggle_dt(&led2);
		
		
		if (ret < 0) {
			return 0;
		}

		k_msleep(SLEEP_TIME_MS);
		led_state = !led_state;
		
		printf("Hello - Counter = %d\n",counter);
		

		if( counter > 10) reset_to_uf2();
		else counter++;

	}
	return 0;
}