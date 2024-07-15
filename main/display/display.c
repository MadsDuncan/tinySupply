#include "esp_timer.h"
#include "lvgl_helpers.h"
#include <stdint.h>

#include "display.h"
#include "display_internal.h"

#define LV_TICK_PERIOD_MS 1

// Semaphore to handle concurrent call to lvgl stuff
// If you wish to call *any* lvgl function from other threads/tasks
// you should lock on the very same semaphore!
SemaphoreHandle_t lv_task_sema;

lv_indev_t *indev_touchpad;
lv_indev_t *indev_encoder;

static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void lv_task(void *pvParameter) {
    (void) pvParameter;
    lv_task_sema = xSemaphoreCreateMutex();

    // Initialize library
    lv_init();

    // Initialize SPI and I2C
    lvgl_driver_init();
    display_drv_encoder_setup();

    // Buffer setup
    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);

    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);

    // Display setup
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.hor_res = CONFIG_LV_HOR_RES_MAX;
    disp_drv.ver_res = CONFIG_LV_VER_RES_MAX;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    // Touch interface setup
    lv_indev_drv_t indev_drv_touch;
    lv_indev_drv_init(&indev_drv_touch);
    indev_drv_touch.read_cb = touch_driver_read;
    indev_drv_touch.type = LV_INDEV_TYPE_POINTER;
    indev_touchpad = lv_indev_drv_register(&indev_drv_touch);

    // Encoder setup
    lv_indev_drv_t indev_drv_encoder;
    lv_indev_drv_init(&indev_drv_encoder);
    indev_drv_encoder.type = LV_INDEV_TYPE_ENCODER;
    indev_drv_encoder.read_cb = display_drv_encoder_cb;
    indev_encoder = lv_indev_drv_register(&indev_drv_encoder);

    // Create and start a periodic timer interrupt to call lv_tick_inc
    const esp_timer_create_args_t lv_tick_timer_args = {
        .callback = &lv_tick_task,
        .name = "lv_tick"
    };
    esp_timer_handle_t lv_tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&lv_tick_timer_args, &lv_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lv_tick_timer, LV_TICK_PERIOD_MS * 1000));

    while (1) {
        // Delay 1 tick (assumes FreeRTOS tick is 10ms
        vTaskDelay(pdMS_TO_TICKS(10));

        // Try to take the semaphore, call lvgl related function on success
        if (pdTRUE == xSemaphoreTake(lv_task_sema, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(lv_task_sema);
        }
    }

    // Should never return
    free(buf1);
    free(buf2);
    vTaskDelete(NULL);
}

void display_setup() {
    // If you want to use a task to create the graphic, you NEED to create a Pinned task
    // Otherwise there can be problem such as memory corruption and so on.
    // NOTE: When not using Wi-Fi nor Bluetooth you can pin the lv_task to core 0
    xTaskCreatePinnedToCore(lv_task, "gui", 4096*2, NULL, 0, NULL, 1);
}
