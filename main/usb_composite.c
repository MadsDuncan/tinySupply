#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "tusb_console.h"
#include "tusb_msc_storage.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>

#include "usb_composite.h"

#define BASE_PATH "/usb" // base path to mount the partition

// NOTE: Espressif tinyusb abstraction layer sets a callback that exposes USB storage to the host PC on connection,
// which doesn't work for this application. Therefore, its necessary to comment out the content of tud_mount_cb in
// tusb_msc_storage.c for now.

static bool file_exists(const char *file_path) {
    struct stat buffer;
    return stat(file_path, &buffer) == 0;
}

void usb_comp_setup() {
    printf("Initializing wear leveling\n");
    wl_handle_t wl_handle = WL_INVALID_HANDLE;
    const esp_partition_t *data_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, NULL);
    if (data_partition == NULL) {
        printf("Failed to find FATFS partition. Check the partition table.\n");
        ESP_ERROR_CHECK(ESP_ERR_NOT_FOUND);
    }
    ESP_ERROR_CHECK(wl_mount(data_partition, &wl_handle));

    const tinyusb_msc_spiflash_config_t config_spi = {
        .wl_handle = wl_handle
    };
    ESP_ERROR_CHECK(tinyusb_msc_storage_init_spiflash(&config_spi));
    ESP_ERROR_CHECK(tinyusb_msc_storage_mount(BASE_PATH));

    printf("USB Composite initialization\n");
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .string_descriptor_count = 0,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = NULL,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL
    };
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    printf("USB Composite initialization DONE\n");

    // TODO: Console over TinyUSB CDC
    // This does not work with esp_console REPL out of the box  
    //printf("Switching serial communication to USB port\n");
    //esp_tusb_init_console(TINYUSB_CDC_ACM_0);
    //tinyusb_cdc_init()?
}

void usb_comp_msc_mount() {
    tinyusb_msc_storage_unmount();
}

void usb_comp_msc_unmount() {
    ESP_ERROR_CHECK(tinyusb_msc_storage_mount(BASE_PATH));
}

void usb_comp_msc_test() {
    const char *directory = "/usb/esp";
    const char *file_path = "/usb/esp/test.txt";

    struct stat s = {0};
    bool directory_exists = stat(directory, &s) == 0;
    if (!directory_exists) {
        if (mkdir(directory, 0775) != 0) {
            printf("mkdir failed with errno: %s\n", strerror(errno));
        }
    }

    if (!file_exists(file_path)) {
        printf("Creating file\n");
        FILE *f = fopen(file_path, "w");
        if (f == NULL) {
            printf("Failed to open file for writing\n");
            return;
        }
        fprintf(f, "Hello World!\n");
        fclose(f);
    }

    FILE *f;
    printf("Reading file\n");
    f = fopen(file_path, "r");
    if (f == NULL) {
        printf("Failed to open file for reading\n");
        return;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    printf("Read from file: '%s'\n", line);
}
