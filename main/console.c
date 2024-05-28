#include "esp_console.h"
#include "usb_composite.h"
#include <stdint.h>
#include <stdio.h>

#include "console.h"

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

void console_setup() {
    esp_console_register_help_command();
    register_msc_commands();

    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "tinySupply>";
    repl_config.max_cmdline_length = 120;

    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
