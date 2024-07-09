#include "demos/lv_demos.h"
#include "display.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "lvgl_helpers.h"
#include "sdkconfig.h"
#include <stdint.h>

#define TAG "display"
#define LV_TICK_PERIOD_MS 1

// Semaphore to handle concurrent call to lvgl stuff
// If you wish to call *any* lvgl function from other threads/tasks
// you should lock on the very same semaphore!
SemaphoreHandle_t lv_task_sema;

/************************************************
 *      Setup
 ***********************************************/
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
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);

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

/************************************************
 *      Demos
 ***********************************************/
static uint8_t active_demo = DISPLAY_DEMO_NONE;

static lv_style_t style0, style1;
static lv_obj_t *scr = NULL;
static lv_obj_t *obj = NULL;
static lv_timer_t *timer = NULL;

static void hello_world_demo() {
    scr = lv_disp_get_scr_act(NULL);
    obj =  lv_label_create(scr);
    lv_label_set_text(obj, "Hello\nworld");
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
}

static void full_screen_demo_cb() {
    static bool switch_color = false;
    switch_color = !switch_color;

    if (switch_color) {
        lv_obj_remove_style(scr, &style0, LV_PART_ANY | LV_STATE_ANY);
        lv_obj_add_style(scr, &style1, 0);
    } else {
        lv_obj_remove_style(scr, &style1, LV_PART_ANY | LV_STATE_ANY);
        lv_obj_add_style(scr, &style0, 0);
    }
}

static void full_screen_demo() {
    lv_style_init(&style0);
    lv_style_set_bg_color(&style0, lv_color_hex(0xffffff));
    lv_style_set_text_color(&style0, lv_color_hex(0x000000));

    lv_style_init(&style1);
    lv_style_set_bg_color(&style1, lv_color_hex(0x000000));
    lv_style_set_text_color(&style1, lv_color_hex(0xffffff));

    scr = lv_disp_get_scr_act(NULL);
    obj =  lv_label_create(scr);
    lv_label_set_text(obj, "Hello\nworld");
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_style(scr, &style0, 0);

    timer = lv_timer_create(full_screen_demo_cb, 2000, NULL);    
}

static void thin_rect_demo_cb() {
    static bool switch_color = false;
    switch_color = !switch_color;

    if (switch_color) {
        lv_obj_remove_style(obj, &style0, LV_PART_ANY | LV_STATE_ANY);
        lv_obj_add_style(obj, &style1, 0);
    } else {
        lv_obj_remove_style(obj, &style1, LV_PART_ANY | LV_STATE_ANY);
        lv_obj_add_style(obj, &style0, 0);
    }
}

static void thin_rect_demo() {
    lv_style_init(&style0);
    lv_style_set_bg_color(&style0, lv_color_hex(0x00ff00));

    lv_style_init(&style1);
    lv_style_set_bg_color(&style1, lv_color_hex(0x0000ff));

    scr = lv_disp_get_scr_act(NULL);
    obj = lv_obj_create(scr);
    lv_obj_set_size(obj , 50, 320);
    lv_obj_set_pos(obj , 0, 0);

    lv_obj_add_style(obj, &style0, 0);

    timer = lv_timer_create(thin_rect_demo_cb, 2000, NULL);
}

void display_start_demo(uint8_t demo) {
    if (active_demo != DISPLAY_DEMO_NONE) {
        printf("Already running demo %d\n", active_demo);
        return;
    }

    if (lv_task_sema == NULL) {
        ESP_LOGE(TAG, "Display semaphore not Initialized\n");
        return;
    }

    if (xSemaphoreTake(lv_task_sema, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Display semaphore not free\n");
        return;
    }

    active_demo = demo;

    switch (demo) {
        case DISPLAY_DEMO_LVGL:
            #if defined CONFIG_LV_USE_DEMO_WIDGETS
            lv_demo_widgets();
            #elif CONFIG_LV_USE_DEMO_BENCHMARK
            lv_demo_benchmark();
            #elif CONFIG_LV_USE_DEMO_STRESS
            lv_demo_stress();
            #elif CONFIG_LV_USE_DEMO_KEYPAD_AND_ENCODER
            lv_demo_keypad_encoder();
            #elif CONFIG_LV_USE_DEMO_MUSIC
            lv_demo_music();
            #else
            active_demo = DISPLAY_DEMO_NONE;
            printf("No LVGL demo selected\n");
            #endif
            break;
        case DISPLAY_DEMO_HELLO_WORLD:
            hello_world_demo();
            break;
        case DISPLAY_DEMO_FULL_SCREEN:
            full_screen_demo();
            break;
        case DISPLAY_DEMO_THIN_RECT:
            thin_rect_demo();
            break;
        default:
            active_demo = DISPLAY_DEMO_NONE;
            printf("No demo with number %d", demo);
    }

    xSemaphoreGive(lv_task_sema);    
}

void display_stop_demo() {
    if (lv_task_sema == NULL) {
        ESP_LOGE(TAG, "Display semaphore not Initialized\n");        
        return;
    }

    if (xSemaphoreTake(lv_task_sema, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Display semaphore not free\n");
        return;
    }

    switch (active_demo) {
        case DISPLAY_DEMO_LVGL:
            #if defined CONFIG_LV_USE_DEMO_WIDGETS
            lv_demo_widgets_close();
            #elif CONFIG_LV_USE_DEMO_BENCHMARK
            lv_demo_benchmark_close();
            #elif CONFIG_LV_USE_DEMO_STRESS
            lv_demo_stress_close();
            #elif CONFIG_LV_USE_DEMO_KEYPAD_AND_ENCODER
            lv_demo_keypad_encoder_close();
            #elif CONFIG_LV_USE_DEMO_MUSIC
            lv_demo_music_close();
            #endif
            break;
        case DISPLAY_DEMO_HELLO_WORLD:
            lv_obj_clean(lv_scr_act());
            break;
        case DISPLAY_DEMO_FULL_SCREEN: // Fall through
        case DISPLAY_DEMO_THIN_RECT:
            lv_timer_del(timer);
            timer = NULL;
            lv_style_reset(&style0);
            lv_style_reset(&style1);
            lv_obj_clean(lv_scr_act());
            break;
    }

    active_demo = DISPLAY_DEMO_NONE;

    xSemaphoreGive(lv_task_sema);    
}
