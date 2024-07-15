#include "demos/lv_demos.h"
#include "display.h"
#include "display_drv_encoder.h"
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

lv_indev_t *indev_touchpad;
lv_indev_t *indev_encoder;

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

#include "esp_random.h"

lv_obj_t *v_val_label;
lv_obj_t *i_val_label;
lv_obj_t *v_const_cont;
lv_obj_t *i_const_cont;

static void app_demo_cb() {
    uint32_t mv = esp_random() % 25000;
    uint32_t ma = esp_random() % 1500;

    lv_label_set_text_fmt(v_val_label, "%d.%03d", (int)(mv/1000), (int)(mv%1000));
    lv_label_set_text_fmt(i_val_label, "%d.%03d", (int)(ma/1000), (int)(ma%1000));

    if (esp_random() % 2) {
        lv_obj_add_flag(v_const_cont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(i_const_cont, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(i_const_cont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(v_const_cont, LV_OBJ_FLAG_HIDDEN);
    }
}

static void app_demo() {
    static lv_style_t style_base;
    lv_style_init(&style_base);
    lv_style_set_bg_color(&style_base, lv_color_hex(0x000000));
    lv_style_set_text_color(&style_base, lv_color_hex(0xffffff));
    lv_style_set_text_font(&style_base, &lv_font_montserrat_14);
    lv_style_set_pad_top(&style_base, 0);
    lv_style_set_pad_bottom(&style_base, 0);
    lv_style_set_pad_left(&style_base, 0);
    lv_style_set_pad_right(&style_base, 0);
    lv_style_set_pad_row(&style_base, 0);
    lv_style_set_pad_column(&style_base, 0);
    lv_style_set_border_width(&style_base, 0);
#if 0
    lv_style_set_border_width(&style_base, 1);
    lv_style_set_border_color(&style_base, lv_color_hex(0xff0000));
#endif

    static lv_style_t style_font_small;
    lv_style_set_text_font(&style_font_small, &lv_font_montserrat_20);

    static lv_style_t style_font_mid;
    lv_style_set_text_font(&style_font_mid, &lv_font_montserrat_32);

    static lv_style_t style_font_big;
    lv_style_set_text_font(&style_font_big, &lv_font_montserrat_48);

    static lv_style_t style_cv;
    lv_style_set_bg_color(&style_cv, lv_color_hex(0xff0000));
    lv_style_set_text_color(&style_cv, lv_color_hex(0x000000));

    static lv_style_t style_cc;
    lv_style_set_bg_color(&style_cc, lv_color_hex(0x0000ff));
    lv_style_set_text_color(&style_cc, lv_color_hex(0x000000));

    scr = lv_disp_get_scr_act(NULL);
    lv_obj_add_style(scr, &style_base, 0);

    // voltage fields
    lv_obj_t *v_grid = lv_obj_create(scr);
    lv_obj_set_height(v_grid, LV_SIZE_CONTENT);
    lv_obj_add_style(v_grid, &style_base, 0);

    lv_obj_t *v_set_str_cont = lv_obj_create(v_grid);
    lv_obj_add_style(v_set_str_cont, &style_base, 0);
    lv_obj_t *v_set_str_label = lv_label_create(v_set_str_cont);
    lv_obj_add_style(v_set_str_label, &style_base, 0);
    lv_obj_add_style(v_set_str_label, &style_font_mid, 0);
    lv_obj_set_align(v_set_str_label, LV_ALIGN_CENTER);
    lv_label_set_text(v_set_str_label, "V set");

    lv_obj_t *v_set_cont = lv_obj_create(v_grid);
    lv_obj_add_style(v_set_cont, &style_base, 0);
    lv_obj_t *v_set_label = lv_label_create(v_set_cont);
    lv_obj_add_style(v_set_label, &style_base, 0);
    lv_obj_add_style(v_set_label, &style_font_mid, 0);
    lv_obj_set_align(v_set_label, LV_ALIGN_CENTER);
    lv_label_set_text(v_set_label, "12.000");

    lv_obj_t *v_val_cont = lv_obj_create(v_grid);
    lv_obj_add_style(v_val_cont, &style_base, 0);
    v_val_label = lv_label_create(v_val_cont);
    lv_obj_add_style(v_val_label, &style_base, 0);
    lv_obj_add_style(v_val_label, &style_font_big, 0);
    lv_obj_set_align(v_val_label, LV_ALIGN_CENTER);
    lv_label_set_text(v_val_label, "12.232");
    
    v_const_cont = lv_obj_create(v_grid);
    lv_obj_add_style(v_const_cont, &style_base, 0);
    lv_obj_add_style(v_const_cont, &style_cv, 0);
    lv_obj_add_flag(v_const_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *v_const_label = lv_label_create(v_const_cont);
    lv_obj_add_style(v_const_label, &style_base, 0);
    lv_obj_add_style(v_const_label, &style_cv, 0);
    lv_obj_add_style(v_const_label, &style_font_mid, 0);
    lv_obj_set_align(v_const_label, LV_ALIGN_CENTER);
    lv_label_set_text(v_const_label, "CV");

    lv_obj_t *v_unit_cont = lv_obj_create(v_grid);
    lv_obj_add_style(v_unit_cont, &style_base, 0);
    lv_obj_t *v_unit_label = lv_label_create(v_unit_cont);
    lv_obj_add_style(v_unit_label, &style_base, 0);
    lv_obj_add_style(v_unit_label, &style_font_mid, 0);
    lv_obj_set_align(v_unit_label, LV_ALIGN_CENTER);
    lv_label_set_text(v_unit_label, "V");

    // Current fields
    lv_obj_t *i_grid = lv_obj_create(scr);
    lv_obj_set_height(i_grid, LV_SIZE_CONTENT);
    lv_obj_add_style(i_grid, &style_base, 0);

    lv_obj_t *i_set_str_cont = lv_obj_create(i_grid);
    lv_obj_add_style(i_set_str_cont, &style_base, 0);
    lv_obj_t *i_set_str_label = lv_label_create(i_set_str_cont);
    lv_obj_add_style(i_set_str_label, &style_base, 0);
    lv_obj_add_style(i_set_str_label, &style_font_mid, 0);
    lv_obj_set_align(i_set_str_label, LV_ALIGN_CENTER);
    lv_label_set_text(i_set_str_label, "I set");

    lv_obj_t *i_set_cont = lv_obj_create(i_grid);
    lv_obj_add_style(i_set_cont, &style_base, 0);
    lv_obj_t *i_set_label = lv_label_create(i_set_cont);
    lv_obj_add_style(i_set_label, &style_base, 0);
    lv_obj_add_style(i_set_label, &style_font_mid, 0);
    lv_obj_set_align(i_set_label, LV_ALIGN_CENTER);
    lv_label_set_text(i_set_label, "1.000");

    lv_obj_t *i_val_cont = lv_obj_create(i_grid);
    lv_obj_add_style(i_val_cont, &style_base, 0);
    i_val_label = lv_label_create(i_val_cont);
    lv_obj_add_style(i_val_label, &style_base, 0);
    lv_obj_add_style(i_val_label, &style_font_big, 0);
    lv_obj_set_align(i_val_label, LV_ALIGN_CENTER);
    lv_label_set_text(i_val_label, "0.990");
    
    i_const_cont = lv_obj_create(i_grid);
    lv_obj_add_style(i_const_cont, &style_base, 0);
    lv_obj_add_style(i_const_cont, &style_cc, 0);
    lv_obj_add_flag(i_const_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *i_const_label = lv_label_create(i_const_cont);
    lv_obj_add_style(i_const_label, &style_base, 0);
    lv_obj_add_style(i_const_label, &style_cc, 0);
    lv_obj_add_style(i_const_label, &style_font_mid, 0);
    lv_obj_set_align(i_const_label, LV_ALIGN_CENTER);
    lv_label_set_text(i_const_label, "CC");

    lv_obj_t *i_unit_cont = lv_obj_create(i_grid);
    lv_obj_add_style(i_unit_cont, &style_base, 0);
    lv_obj_t *i_unit_label = lv_label_create(i_unit_cont);
    lv_obj_add_style(i_unit_label, &style_base, 0);
    lv_obj_add_style(i_unit_label, &style_font_mid, 0);
    lv_obj_set_align(i_unit_label, LV_ALIGN_CENTER);
    lv_label_set_text(i_unit_label, "A");

    // Top grid
    static lv_coord_t main_grid_col[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t main_grid_row[] = {130, LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

    // Grid for voltage and current
    static lv_coord_t param_grid_col[] = {150, LV_GRID_FR(1), 50, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t param_grid_row[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};


    lv_obj_set_grid_dsc_array(scr, main_grid_col, main_grid_row);
    lv_obj_set_grid_cell(v_grid, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_grid_cell(i_grid, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);

    lv_obj_set_grid_dsc_array(v_grid, param_grid_col, param_grid_row);
    lv_obj_set_grid_cell(v_set_str_cont, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(v_set_cont,     LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_grid_cell(v_val_cont,     LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
    lv_obj_set_grid_cell(v_const_cont,   LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(v_unit_cont,    LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_set_grid_dsc_array(i_grid, param_grid_col, param_grid_row);
    lv_obj_set_grid_cell(i_set_str_cont, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(i_set_cont,     LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_grid_cell(i_val_cont,     LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
    lv_obj_set_grid_cell(i_const_cont,   LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(i_unit_cont,    LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    timer = lv_timer_create(app_demo_cb, 500, NULL);
}

static lv_obj_t * spinbox;

static void spinbox_inc_demo_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_SHORT_CLICKED || code  == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(spinbox);
    }
}

static void spinbox_dec_demo_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(spinbox);
    }
}

static void spinbox_demo() {
    spinbox = lv_spinbox_create(lv_scr_act());
    lv_spinbox_set_range(spinbox, 0, 25000);
    lv_spinbox_set_digit_format(spinbox, 5, 2);
    lv_spinbox_step_prev(spinbox);
    lv_obj_set_width(spinbox, 200);
    lv_obj_center(spinbox);
    lv_obj_set_style_text_font(spinbox, &lv_font_montserrat_48, 0);

    lv_coord_t h = lv_obj_get_height(spinbox);

    lv_obj_t *btn_p = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_p, h, h);
    lv_obj_align_to(btn_p, spinbox, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_bg_img_src(btn_p, LV_SYMBOL_PLUS, 0);
    lv_obj_add_event_cb(btn_p, spinbox_inc_demo_cb, LV_EVENT_ALL,  NULL);

    lv_obj_t *btn_m = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_m, h, h);
    lv_obj_align_to(btn_m, spinbox, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    lv_obj_set_style_bg_img_src(btn_m, LV_SYMBOL_MINUS, 0);
    lv_obj_add_event_cb(btn_m, spinbox_dec_demo_cb, LV_EVENT_ALL, NULL);

    lv_group_t *group = lv_group_create();
    lv_group_add_obj(group, spinbox);
    lv_group_add_obj(group, btn_p);
    lv_group_add_obj(group, btn_m);
    lv_indev_set_group(indev_encoder, group);
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
        case DISPLAY_DEMO_APP:
            app_demo();
            break;
        case DISPLAY_DEMO_SPINBOX:
            spinbox_demo();
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
