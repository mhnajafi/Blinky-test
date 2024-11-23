#include "spi_flash.h"
#include <zephyr/logging/log.h>
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
// #include <zephyr/zephyr.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>
#include <string.h>
#include <stdio.h>
// #include "../src/naya_connect/naya_con_init.h"
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>

LOG_MODULE_REGISTER(spi_flash, LOG_LEVEL_DEBUG);

// Define the flash area ID using the partition label
#define LAYOUT_PARTITION_ID FIXED_PARTITION_ID(layout_partition)
#define LOG_PARTITION_ID FIXED_PARTITION_ID(log_partition)

#define M_FIRMWARE_PARTITION_ID FIXED_PARTITION_ID(slot2_partition)

// Partition sizes
#define LAYOUT_PARTITION_SIZE (1 * 1024 * 1024)       // 1MB
#define LOG_PARTITION_SIZE (0.5 * 1024 * 1024)        // 0.5 MB

#define M_FIRMWARE_PARTITION_SIZE (1 * 1024 * 1024) // 1.5 MB
#define FLASH_PARTITION_SIZE                                                                       \
    (LAYOUT_PARTITION_SIZE + LOG_PARTITION_SIZE + RES_PARTITION_SIZE +                             \
     M_FIRMWARE_PARTITION_SIZE) // 4MB

#define BLOCK_SIZE 4096

#define LAYOUT_PARTITION_BLOCK_COUNT (LAYOUT_PARTITION_SIZE / BLOCK_SIZE)         // 256
#define LOG_PARTITION_BLOCK_COUNT (LOG_PARTITION_SIZE / BLOCK_SIZE)               // 128

#define M_FIRMWARE_PARTITION_BLOCK_COUNT (M_FIRMWARE_PARTITION_SIZE / BLOCK_SIZE) // 384

#define BLOCK_COUNT (FLASH_PARTITION_SIZE / BLOCK_SIZE) // 1024

// Global flash area pointers to maintain a single open flash area
const struct flash_area *fa_layout_partition;
const struct flash_area *fa_log_partition;

const struct flash_area *fa_m_firmware_partition;

// SPI flash status variables for each partition
enum spi_flash_status layout_partition_status = SPI_FLASH_NOT_DETECTED;
enum spi_flash_status log_partition_status = SPI_FLASH_NOT_DETECTED;

enum spi_flash_status m_firmware_partition_status = SPI_FLASH_NOT_DETECTED;

// Mutex for synchronizing SPI flash access
struct k_mutex spi_flash_mutex;

// Forward declarations of custom flash operations
int layout_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset, void *buffer,
                      lfs_size_t size);
int layout_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
                      const void *buffer, lfs_size_t size);
int layout_flash_erase(const struct lfs_config *c, lfs_block_t block);
int custom_flash_sync(const struct lfs_config *c);

// Custom flash operations for log partition
int log_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset, void *buffer,
                   lfs_size_t size);
int log_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
                   const void *buffer, lfs_size_t size);
int log_flash_erase(const struct lfs_config *c, lfs_block_t block);


// Custom flash operations for m_firmware partition
int m_firmware_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
                          void *buffer, lfs_size_t size);
int m_firmware_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
                          const void *buffer, lfs_size_t size);
int m_firmware_flash_erase(const struct lfs_config *c, lfs_block_t block);

// Updated LittleFS configurations for each partition
const struct lfs_config layout_partition_cfg = {
    .read = layout_flash_read,
    .prog = layout_flash_prog,
    .erase = layout_flash_erase,
    .sync = custom_flash_sync,
    .read_size = 256,
    .prog_size = 256,
    .block_size = BLOCK_SIZE,
    .block_count = LAYOUT_PARTITION_BLOCK_COUNT,
    .cache_size = 256,
    .lookahead_size = 256,
    .block_cycles = 500,
};

const struct lfs_config log_partition_cfg = {
    .read = log_flash_read,
    .prog = log_flash_prog,
    .erase = log_flash_erase,
    .sync = custom_flash_sync,
    .read_size = 256,
    .prog_size = 256,
    .block_size = BLOCK_SIZE,
    .block_count = LOG_PARTITION_BLOCK_COUNT,
    .cache_size = 256,
    .lookahead_size = 256,
    .block_cycles = 500,
};



const struct lfs_config m_firmware_partition_cfg = {
    .read = m_firmware_flash_read,
    .prog = m_firmware_flash_prog,
    .erase = m_firmware_flash_erase,
    .sync = custom_flash_sync,
    .read_size = 256,
    .prog_size = 256,
    .block_size = BLOCK_SIZE,
    .block_count = M_FIRMWARE_PARTITION_BLOCK_COUNT,
    .cache_size = 256,
    .lookahead_size = 256,
    .block_cycles = 500,
};

// Declare lfs_t objects to hold the LittleFS state
struct lfs layout_littlefs;
struct lfs log_littlefs;

struct lfs m_firmware_littlefs;

// Adjust the mount point configuration to use your custom lfs_config
static struct fs_mount_t layout_mount_point = {
    .type = FS_LITTLEFS,
    .mnt_point = "/lyt",
    .fs_data = (void *)&layout_partition_cfg,
    .storage_dev = NULL,
};

static struct fs_mount_t log_mount_point = {
    .type = FS_LITTLEFS,
    .mnt_point = "/log",
    .fs_data = (void *)&log_partition_cfg,
    .storage_dev = NULL,
};


static struct fs_mount_t m_firmware_mount_point = {
    .type = FS_LITTLEFS,
    .mnt_point = "/mfmw",
    .fs_data = (void *)&m_firmware_partition_cfg,
    .storage_dev = NULL,
};

/**
 * @brief Reads data from the layout partition flash.
 *
 * This function reads data from the specified block and offset within the layout partition.
 *
 * @param c      The lfs_config structure.
 * @param block  The block number to read from.
 * @param offset The offset within the block.
 * @param buffer The buffer to store the read data.
 * @param size   The number of bytes to read.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int layout_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset, void *buffer,
                      lfs_size_t size) {
    int rc;

    // Lock the mutex to prevent concurrent access
    k_mutex_lock(&spi_flash_mutex, K_FOREVER);

    rc = flash_area_read(fa_layout_partition, (block * c->block_size) + offset, buffer, size);

    // Unlock the mutex
    k_mutex_unlock(&spi_flash_mutex);

    if (rc < 0) {
        LOG_ERR("Failed to read block %d at offset %d: %d", block, offset, rc);
        printf( "Failed to read block %d at offset %d: %d\n\r", block, offset, rc);
    }

    return rc;
}

/**
 * @brief Programs data to the layout partition flash.
 *
 * This function writes data to the specified block and offset within the layout partition.
 *
 * @param c      The lfs_config structure.
 * @param block  The block number to write to.
 * @param offset The offset within the block.
 * @param buffer The data buffer to write.
 * @param size   The number of bytes to write.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int layout_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
                      const void *buffer, lfs_size_t size) {
    int rc;

    // Lock the mutex to prevent concurrent access
    k_mutex_lock(&spi_flash_mutex, K_FOREVER);

    rc = flash_area_write(fa_layout_partition, (block * c->block_size) + offset, buffer, size);

    // Unlock the mutex
    k_mutex_unlock(&spi_flash_mutex);

    if (rc < 0) {
        LOG_ERR("Failed to program block %d at offset %d: %d", block, offset, rc);
        printf("Failed to program block %d at offset %d: %d\n\r", block, offset, rc);
    }

    return rc;
}

/**
 * @brief Erases a block in the layout partition flash.
 *
 * @param c     The lfs_config structure.
 * @param block The block number to erase.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int layout_flash_erase(const struct lfs_config *c, lfs_block_t block) {
    int rc;
    uint32_t offset = block * c->block_size;
    uint32_t size = c->block_size;

    // Lock the mutex to prevent concurrent access
    k_mutex_lock(&spi_flash_mutex, K_FOREVER);

    rc = flash_area_erase(fa_layout_partition, offset, size);

    // Unlock the mutex
    k_mutex_unlock(&spi_flash_mutex);

    if (rc < 0) {
        LOG_ERR("Failed to erase block %d: %d", block, rc);
        printf( "Failed to erase block %d: %d\n\r", block,rc);
    }

    return rc;
}

/**
 * @brief Synchronizes the flash (no operation needed for most flash devices).
 *
 * @param c The lfs_config structure.
 *
 * @return 0 on success.
 */
int custom_flash_sync(const struct lfs_config *c) {
    // No sync operation needed for most flash devices
    return 0;
}

// Implementations for log partition flash operations
int log_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset, void *buffer,
                   lfs_size_t size) {
    int rc;

    k_mutex_lock(&spi_flash_mutex, K_FOREVER);
    rc = flash_area_read(fa_log_partition, (block * c->block_size) + offset, buffer, size);
    k_mutex_unlock(&spi_flash_mutex);

    if (rc < 0) {
        LOG_ERR("Log: Failed to read block %d at offset %d: %d", block, offset, rc);
    }

    return rc;
}

int log_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
                   const void *buffer, lfs_size_t size) {
    int rc;

    k_mutex_lock(&spi_flash_mutex, K_FOREVER);
    rc = flash_area_write(fa_log_partition, (block * c->block_size) + offset, buffer, size);
    k_mutex_unlock(&spi_flash_mutex);

    if (rc < 0) {
        LOG_ERR("Log: Failed to program block %d at offset %d: %d", block, offset, rc);
    }

    return rc;
}

int log_flash_erase(const struct lfs_config *c, lfs_block_t block) {
    int rc;
    uint32_t offset = block * c->block_size;
    uint32_t size = c->block_size;

    k_mutex_lock(&spi_flash_mutex, K_FOREVER);
    rc = flash_area_erase(fa_log_partition, offset, size);
    k_mutex_unlock(&spi_flash_mutex);

    if (rc < 0) {
        LOG_ERR("Log: Failed to erase block %d: %d", block, rc);
    }

    return rc;
}




// Implementations for m_firmware partition flash operations
int m_firmware_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
                          void *buffer, lfs_size_t size) {
    int rc;

    k_mutex_lock(&spi_flash_mutex, K_FOREVER);
    rc = flash_area_read(fa_m_firmware_partition, (block * c->block_size) + offset, buffer, size);
    k_mutex_unlock(&spi_flash_mutex);

    if (rc < 0) {
        LOG_ERR("M_Firmware: Failed to read block %d at offset %d: %d", block, offset, rc);
    }

    return rc;
}

int m_firmware_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
                          const void *buffer, lfs_size_t size) {
    int rc;

    k_mutex_lock(&spi_flash_mutex, K_FOREVER);
    rc = flash_area_write(fa_m_firmware_partition, (block * c->block_size) + offset, buffer, size);
    k_mutex_unlock(&spi_flash_mutex);

    if (rc < 0) {
        LOG_ERR("M_Firmware: Failed to program block %d at offset %d: %d", block, offset, rc);
    }

    return rc;
}

int m_firmware_flash_erase(const struct lfs_config *c, lfs_block_t block) {
    int rc;
    uint32_t offset = block * c->block_size;
    uint32_t size = c->block_size;

    k_mutex_lock(&spi_flash_mutex, K_FOREVER);
    rc = flash_area_erase(fa_m_firmware_partition, offset, size);
    k_mutex_unlock(&spi_flash_mutex);

    if (rc < 0) {
        LOG_ERR("M_Firmware: Failed to erase block %d: %d", block, rc);
    }

    return rc;
}

/**
 * @brief Initializes the SPI flash and mounts partitions.
 *
 * This function initializes the SPI flash, opens flash areas for each partition,
 * and mounts the partitions. It also handles formatting if partitions are not formatted.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int spiflash_init(void) {
    int rc;

    // Initialize the SPI flash mutex
    k_mutex_init(&spi_flash_mutex);

    LOG_INF("Initializing SPI Flash...");
    printf( "Initializing SPI Flash...\n\r");

    // Open flash areas for each partition
    rc = flash_area_open(LAYOUT_PARTITION_ID, &fa_layout_partition);
    if (rc < 0) {
        LOG_ERR("Failed to open flash area for layout partition: %d", rc);
        printf("Failed to open flash area for layout partition: %d\n\r", rc);
        return rc;
    }
    layout_partition_status = SPI_FLASH_DETECTED;

    rc = flash_area_open(LOG_PARTITION_ID, &fa_log_partition);
    if (rc < 0) {
        LOG_ERR("Failed to open flash area for log partition: %d", rc);
        printf("Failed to open flash area for log partition: %d\n\r", rc);
        return rc;
    }
    log_partition_status = SPI_FLASH_DETECTED;


    rc = flash_area_open(M_FIRMWARE_PARTITION_ID, &fa_m_firmware_partition);
    if (rc < 0) {
        LOG_ERR("Failed to open flash area for m_firmware partition: %d", rc);
        printf("Failed to open flash area for m_firmware partition: %d\n\r", rc);
        return rc;
    }
    m_firmware_partition_status = SPI_FLASH_DETECTED;

    // Mount the layout partition
    LOG_INF("Attempting to mount Layout_partition...");
    printf( "Attempting to mount Layout_partition...\n\r");
    rc = lfs_mount(&layout_littlefs, &layout_partition_cfg);
    if (rc == 0) {
        LOG_INF("Layout_partition mounted at %s", layout_mount_point.mnt_point);
        printf("Layout_partition mounted at %s\n\r",
                                       layout_mount_point.mnt_point);
        layout_partition_status = SPI_FLASH_MOUNTED;
    } else if (rc == LFS_ERR_CORRUPT) { // LittleFS-specific error code for unformatted partition
        LOG_INF("Formatting Layout_partition...");
        printf( "Formatting Layout_partition...\n\r");

        rc = lfs_format(&layout_littlefs, &layout_partition_cfg);
        if (rc == 0) {
            LOG_INF("Layout_partition formatted successfully, retrying mount...");
            printf(
                                 "Layout_partition formatted successfully, retrying mount...\n\r");
            layout_partition_status = SPI_FLASH_FORMATTED;

            rc = lfs_mount(&layout_littlefs, &layout_partition_cfg);
            if (rc == 0) {
                LOG_INF("Layout_partition mounted successfully after format");
                printf("Layout_partition mounted successfully after format\n\r");
                layout_partition_status = SPI_FLASH_MOUNTED;
            } else {
                LOG_ERR("Failed to mount Layout_partition after format: %d", rc);
                printf("Failed to mount Layout_partition after format: %d\n\r", rc);
            }
        } else {
            LOG_ERR("Failed to format Layout_partition: %d", rc);
            printf("Failed to format Layout_partition: %d\n\r", rc);
        }
    } else {
        LOG_ERR("Failed to mount Layout_partition: %d", rc);
        printf("Failed to mount Layout_partition: %d\n\r",
                                       rc);
    }

    // Similar code for mounting other partitions (log, res, m_firmware)
    // Mount the log partition
    LOG_INF("Attempting to mount Log_partition...");
    printf( "Attempting to mount Log_partition...\n\r");
    rc = lfs_mount(&log_littlefs, &log_partition_cfg);
    if (rc == 0) {
        LOG_INF("Log_partition mounted at %s", log_mount_point.mnt_point);
        printf("Log_partition mounted at %s\n\r",
                                       log_mount_point.mnt_point);
        log_partition_status = SPI_FLASH_MOUNTED;
    } else if (rc == LFS_ERR_CORRUPT) {
        LOG_INF("Formatting Log_partition...");
        printf( "Formatting Log_partition...\n\r");

        rc = lfs_format(&log_littlefs, &log_partition_cfg);
        if (rc == 0) {
            LOG_INF("Log_partition formatted successfully, retrying mount...");
            printf(
                                 "Log_partition formatted successfully, retrying mount...\n\r");
            log_partition_status = SPI_FLASH_FORMATTED;

            rc = lfs_mount(&log_littlefs, &log_partition_cfg);
            if (rc == 0) {
                LOG_INF("Log_partition mounted successfully after format");
                printf("Log_partition mounted successfully after format\n\r");
                log_partition_status = SPI_FLASH_MOUNTED;
            } else {
                LOG_ERR("Failed to mount Log_partition after format: %d", rc);
                printf("Failed to mount Log_partition after format: %d\n\r", rc);
            }
        } else {
            LOG_ERR("Failed to format Log_partition: %d", rc);
            printf( "Failed to format Log_partition: %d\n\r",
                                           rc);
        }
    } else {
        LOG_ERR("Failed to mount Log_partition: %d", rc);
        printf("Failed to mount Log_partition: %d\n\r", rc);
    }

   
    // Mount the m_firmware partition
    LOG_INF("Attempting to mount M_Firmware_partition...");
    printf( "Attempting to mount M_Firmware_partition...\n\r");
    rc = lfs_mount(&m_firmware_littlefs, &m_firmware_partition_cfg);
    if (rc == 0) {
        LOG_INF("M_Firmware_partition mounted at %s", m_firmware_mount_point.mnt_point);
        printf("M_Firmware_partition mounted at %s\n\r",
                                       m_firmware_mount_point.mnt_point);
        m_firmware_partition_status = SPI_FLASH_MOUNTED;
    } else if (rc == LFS_ERR_CORRUPT) {
        LOG_INF("Formatting M_Firmware_partition...");
        printf( "Formatting M_Firmware_partition...\n\r");

        rc = lfs_format(&m_firmware_littlefs, &m_firmware_partition_cfg);
        if (rc == 0) {
            LOG_INF("M_Firmware_partition formatted successfully, retrying mount...");
            printf("M_Firmware_partition formatted successfully, retrying mount...\n\r");
            m_firmware_partition_status = SPI_FLASH_FORMATTED;

            rc = lfs_mount(&m_firmware_littlefs, &m_firmware_partition_cfg);
            if (rc == 0) {
                LOG_INF("M_Firmware_partition mounted successfully after format");
                printf( "M_Firmware_partition mounted successfully after format\n\r");
                m_firmware_partition_status = SPI_FLASH_MOUNTED;
            } else {
                LOG_ERR("Failed to mount M_Firmware_partition after format: %d", rc);
                printf("Failed to mount M_Firmware_partition after format: %d\n\r",
                    rc);
            }
        } else {
            LOG_ERR("Failed to format M_Firmware_partition: %d", rc);
            printf("Failed to format M_Firmware_partition: %d\n\r", rc);
        }
    } else {
        LOG_ERR("Failed to mount M_Firmware_partition: %d", rc);
        printf("Failed to mount M_Firmware_partition: %d\n\r", rc);
    }

    return rc;
}
