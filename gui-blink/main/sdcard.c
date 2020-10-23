/**************************************************
* To do list:
* i. replace every logging function with OLED display
* ii. add function documentation
*
**************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "sdkconfig.h"
#include "../include/sdcard.h"
#include "driver/gpio.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include <string.h>

#define TAG "SD-CARD"

static int is_mounted = 0;

static uint8_t spi_init_flag = 0x0;

esp_err_t spi_init(spi_host_device_t spi_host) {
    // ensure module init once
    if ( ( spi_init_flag & (1 << spi_host) ) != 0 ) {
        ESP_LOGI(TAG, "Already initialized. Aborting attempt.");
        return ESP_OK;
    }

    // default config for SD interface
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    // initialize HSPI bus
    esp_err_t err = spi_bus_initialize(spi_host, &bus_cfg, SPI_DMA_CHANNEL);
    if (err == ESP_OK) {
        spi_init_flag |= (1 << spi_host);
        ESP_LOGI(TAG, "Successfully initialized bus.");
    }
    else
        ESP_LOGE(TAG, "Failed to initialize bus (%s).", esp_err_to_name(err));
    
    return err;
}

esp_err_t mount_fs(const char* mount_point, sdmmc_card_t* sdcard) {
    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 4,
        .allocation_unit_size = 8 * 1024,
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_device_config_t dev_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    dev_config.gpio_cs = PIN_NUM_CS_SD;
    dev_config.host_id = host.slot;

    sleep(3);

    esp_err_t ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &dev_config,
        &mount_config, &sdcard);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Successfully mounted SD card.");
        sdmmc_card_print_info(stdout, sdcard);
        is_mounted = 1;
    }
    else
        ESP_LOGE(TAG, "Failed to mount SD card (%s).", esp_err_to_name(ret));
    
    return ret;
}

// void app_main() {
//     spi_init(SPI2_HOST);
//     mount_fs();

//     ESP_LOGI(TAG, "Opening file");
//     FILE* f = fopen(MOUNT_POINT"/hello.txt", "w");
//     if (f == NULL) {
//         ESP_LOGE(TAG, "Failed to open file for writing");
//         return;
//     }
//     fprintf(f, "Hello %s!\n", sdcard->cid.name);
//     fclose(f);
//     ESP_LOGI(TAG, "File written");
//     sleep(3);
//     esp_vfs_fat_sdcard_unmount(MOUNT_POINT, sdcard);
// }

/*void example(void)
{
    if ( (hspi_init() != ESP_OK) || (mount_fs() != ESP_OK) )
        return;

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Use POSIX and C standard library functions to work with files.
    // First create a file.
    ESP_LOGI(TAG, "Opening file");
    FILE* f = fopen(MOUNT_POINT"/hello.txt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(MOUNT_POINT"/foo.txt", &st) == 0) {
        // Delete it if it exists
        unlink(MOUNT_POINT"/foo.txt");
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file");
    if (rename(MOUNT_POINT"/hello.txt", MOUNT_POINT"/foo.txt") != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file");
    f = fopen(MOUNT_POINT"/foo.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");

    //deinitialize the bus after all devices are removed
    spi_bus_free(host.slot);
}*/