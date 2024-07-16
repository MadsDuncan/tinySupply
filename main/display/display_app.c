#include "esp_random.h"

#include "display.h"
#include "display_internal.h"

static lv_obj_t *scr = NULL;

static lv_obj_t *v_val_label;
static lv_obj_t *i_val_label;
static lv_obj_t *v_const_cont;
static lv_obj_t *i_const_cont;

static void app_cb() {
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

void display_start_app() {
    if (lv_task_sema == NULL) {
        ESP_LOGE(TAG, "Display semaphore not Initialized\n");
        return;
    }

    if (xSemaphoreTake(lv_task_sema, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Display semaphore not free\n");
        return;
    }

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

	lv_timer_t *timer = lv_timer_create(app_cb, 500, NULL);

    xSemaphoreGive(lv_task_sema);
}
