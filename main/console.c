#include "display.h"
#include "encoder.h"
#include "esp_console.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "usb_composite.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "console.h"

/************************************************
 *      USB MSC commands
 ***********************************************/
static int msc_mount(int argc, char **argv) {
    printf("Mounting internal storage to PC\n");
    usb_comp_msc_mount();
    return 0;
}

static int msc_unmount(int argc, char **argv) {
    printf("Unmounting internal storage to PC\n");
    usb_comp_msc_unmount();
    return 0;
}

static int msc_test(int argc, char **argv) {
    printf("MSC test\n");
    usb_comp_msc_test();
    return 0;
}

static void register_msc_commands() {
    const esp_console_cmd_t cmd_mount = {
        .command = "msc_mount",
        .help = "Mount internal storage to PC",
        .hint = NULL,
        .func = &msc_mount,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_mount));

    const esp_console_cmd_t cmd_unmount = {
        .command = "msc_unmount",
        .help = "Unmount internal storage to PC",
        .hint = NULL,
        .func = &msc_unmount,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_unmount));

    const esp_console_cmd_t cmd_test = {
        .command = "msc_test",
        .help = "MSC test",
        .hint = NULL,
        .func = &msc_test,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_test));
}

/************************************************
 *      Encoder commands
 ***********************************************/
#if 0
uint32_t enc_0_test_var = 100;
uint32_t enc_1_test_var = 10;

static int enc_test(int argc, char **argv) {
    printf("Encoder test\n");
    encoder_register_var(ENCODER_0, &enc_0_test_var, 10, 0, 200);
    encoder_register_var(ENCODER_1, &enc_1_test_var, 2, 5, 20);

    uint32_t iterations = 1;

    // At least 1 argument
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations == 0) {
            printf("Invalid 1st argument: %s\n", argv[1]);
            return 1;
        }
    }

    for (uint32_t i = 0; i < iterations; i++) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("Encoder      0      1\n");
        printf("---------------------\n");
        printf("btn        %3d    %3d\n", (int)encoder_get_btn(ENCODER_0),         (int)encoder_get_btn(ENCODER_1));
        printf("btn_sticky %3d    %3d\n", (int)encoder_get_btn_sticky(ENCODER_0),  (int)encoder_get_btn_sticky(ENCODER_1));
        printf("dir_sticky %3d    %3d\n", (int)encoder_get_dir_sticky(ENCODER_0),  (int)encoder_get_dir_sticky(ENCODER_1));
        printf("test_var   %3d    %3d\n", (int)enc_0_test_var,                     (int)enc_1_test_var);
        printf("\n");        
    }

    encoder_unregister_var(ENCODER_0);
    encoder_unregister_var(ENCODER_1);
    return 0;
}

static void register_enc_commands() {
    const esp_console_cmd_t cmd_test = {
        .command = "enc_test",
        .help = "Encoder test",
        .hint = NULL,
        .func = &enc_test,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_test));
}
#endif
/************************************************
 *      Display commands
 ***********************************************/
static int disp_start_test(int argc, char **argv) {
    printf("Starting display demo\n");
    display_start_demo(atoi(argv[1]));
    return 0;
}

static int disp_stop_test(int argc, char **argv) {
    printf("Stopping display demo\n");
    display_stop_demo();
    return 0;
}

static void register_disp_commands() {
    const esp_console_cmd_t cmd_start = {
        .command = "disp_test",
        .help = "Start display demo",
        .hint = NULL,
        .func = &disp_start_test,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_start));

    const esp_console_cmd_t cmd_stop = {
        .command = "disp_test_stop",
        .help = "Stop display demo",
        .hint = NULL,
        .func = &disp_stop_test,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_stop));
}

/************************************************
 *      Public functions
 ***********************************************/
void console_setup() {
    esp_console_register_help_command();
    register_msc_commands();
    //register_enc_commands();
    register_disp_commands();

    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "tinySupply>";
    repl_config.max_cmdline_length = 120;

    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
