/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include "spi_flash.h"

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
	

	// if (ret < 0) {
	// 	return 0;
	// }

	spiflash_init();

	printf("Initializaion complete\n");

	int counter=0;




	lfs_file_t file;

	if (lfs_file_open(&m_firmware_littlefs,&file,"first-fil.txt",LFS_O_RDWR)==0)
	{
		printf("\nFile is exist!\n");

	}
	else
	{
		printf("\nFile is not exist!\n");
	}



	// if (fs_open(&file, "/mfmw/first-file.txt", FS_O_READ) == 0) {
    //     char buffer[64];
    //     ssize_t read_bytes = fs_read(&file, buffer, sizeof(buffer) - 1);
    //     if (read_bytes >= 0) {
    //         buffer[read_bytes] = '\0';  // Null-terminate the read data
    //         printf("Read from file: %s\n", buffer);
    //     }
    //     fs_close(&file);
    // }
	


	while (1) {
		// ret = gpio_pin_toggle_dt(&led);
		// ret = gpio_pin_toggle_dt(&led1);
		// ret = gpio_pin_toggle_dt(&led2);
		
		
		if (ret < 0) {
			return 0;
		}

		led_state = !led_state;
		// printf("LED state: %s\n", led_state ? "ON" : "OFF");
		k_msleep(SLEEP_TIME_MS);
		printf("Hello -- counter = %d\n",counter);
		

		if( counter > 10) reset_to_uf2();
		else counter++;

	}
	return 0;
}