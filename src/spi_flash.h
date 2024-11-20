#pragma once

#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <lfs.h>
#include <zephyr/fs/fs.h>

// SPI flash status enums
enum spi_flash_status {
    SPI_FLASH_NOT_DETECTED,
    SPI_FLASH_DETECTED,
    SPI_FLASH_FORMATTED,
    SPI_FLASH_MOUNTED,
    SPI_FLASH_ERASED,
};

// Extern declarations for lfs_t objects for each partition
extern struct lfs layout_littlefs;
extern struct lfs log_littlefs;
extern struct lfs res_littlefs;
extern struct lfs m_firmware_littlefs;

// Extern declarations for spi_flash_status variables for each partition
extern enum spi_flash_status layout_partition_status;
extern enum spi_flash_status log_partition_status;
extern enum spi_flash_status res_partition_status;
extern enum spi_flash_status m_firmware_partition_status;

/**
 * @brief Initializes the SPI flash and mounts partitions.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int spiflash_init(void);

/**
 * @brief Reads data from the SPI flash at a specific address.
 *
 * @param address The address to read from.
 * @param data    The buffer to store the read data.
 * @param len     The number of bytes to read.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int spi_flash_read(uint32_t address, void *data, size_t len);

/**
 * @brief Writes data to the SPI flash at a specific address.
 *
 * @param address The address to write to.
 * @param data    The data to write.
 * @param len     The number of bytes to write.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int spi_flash_write(uint32_t address, const void *data, size_t len);

/**
 * @brief Erases a section of the SPI flash.
 *
 * @param address The starting address to erase.
 * @param size    The size of the area to erase.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int spi_flash_erase(uint32_t address, size_t size);

/**
 * @brief Sends a buffer over CDC UART.
 *
 * @param label  A label to prefix the buffer output.
 * @param buffer The buffer to send.
 * @param len    The length of the buffer.
 */
void send_buffer_cdc_uart(const char *label, const uint8_t *buffer, size_t len);

/* Declare the external mount configuration */
extern struct fs_mount_t lfs_qspi_mount;

/* Additional function declarations if needed */
// int create_and_write_files(void);

/*

#include "spi_flash.h"

void some_function(void) {
    if (layout_partition_status == SPI_FLASH_MOUNTED) {
        // Perform file operations using layout_littlefs
        lfs_file_t file;
        int rc = lfs_file_open(&layout_littlefs, &file, "myfile.txt", LFS_O_RDWR | LFS_O_CREAT);
        if (rc == 0) {
            // Write to the file
            const char *message = "Hello, World!";
            lfs_file_write(&layout_littlefs, &file, message, strlen(message));
            lfs_file_close(&layout_littlefs, &file);
        } else {
            // Handle error
            LOG_ERR("Failed to open file: %d", rc);
        }
    } else {
        // Handle unmounted partition
        LOG_ERR("Layout partition is not mounted.");
    }
}

*/