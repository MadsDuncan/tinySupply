#include "console.h"
#include "display.h"
#include "encoder.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "psu.h"
#include "usb_composite.h"
#include "usb_pd.h"
#include <stdint.h>
#include <stdio.h>


static void update_display_task(void *pvParameter) {
    while (true) {

        uint8_t pd_v = 0;
        switch (esp_random() % 6) {
            case 0: pd_v = USB_PD_5V; break;
            case 1: pd_v = USB_PD_9V; break;
            case 2: pd_v = USB_PD_12V; break;
            case 3: pd_v = USB_PD_15V; break;
            case 4: pd_v = USB_PD_18V; break;
            case 5: pd_v = USB_PD_20V; break;
        }

        #if 1 // Ramp function
            static uint32_t v = 0;
            static uint32_t i = 0;
            v += V_MAX/200;
            i += (I_MAX/200)*0.8;
            if (v > V_MAX) {
                v = 0;
                i = 0;
            }
        #else
            uint32_t v = esp_random() % V_MAX; // Voltage in mV
            uint32_t i = esp_random() % I_MAX; // Current in mA
        #endif

        bool const_i = (esp_random() % 2) ? true : false;

        display_update(v, i, const_i, pd_v);

        vTaskDelay(DISPLAY_UPDATE_PERIOD / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
    printf("Booted!\n");
    usb_comp_setup();
    usb_comp_msc_test();
    //encoder_setup();

    display_setup();
    // TODO Fix core 0 panic if no delay before starting app
    vTaskDelay(10 / portTICK_PERIOD_MS);
    display_start_app();
    xTaskCreatePinnedToCore(update_display_task, "update_display", 4096, NULL, 0, NULL, 1);

    console_setup();

    while(1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
