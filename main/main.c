#include "console.h"
#include "encoder.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "usb_composite.h"
#include <stdint.h>
#include <stdio.h>

void app_main(void) {
    printf("Booted!\n");
    usb_comp_setup();
    usb_comp_msc_test();
    console_setup();
    encoder_setup();

    while(1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
