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
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED2_NODE, gpios);


void reset_to_uf2(void) {
  NRF_POWER->GPREGRET = 0x48; // 0xA8 OTA, 0x4e Serial
  NVIC_SystemReset();         // or sd_nvic_SystemReset();
}


int main(void)
{
	int ret;
	bool led_state = true;

	printf("Start of Program : LED Green Blinky \n");


	if (!gpio_is_ready_dt(&led))  return 0;


	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) return 0;


	printf("Initializaion complete\n");

	uint8_t counter=0;


	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) return 0;
		
		led_state = !led_state;
		printf("LED state: %s\n", led_state ? "ON" : "OFF");
		k_msleep(SLEEP_TIME_MS);

		if( counter > 10) reset_to_uf2();
		else counter++;
	}
	return 0;
}