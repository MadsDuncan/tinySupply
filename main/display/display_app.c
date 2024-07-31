#include "esp_random.h"

#include "display.h"
#include "display_internal.h"

#define MILLI_V_MAX 25000
#define MILLI_A_MAX 1500

static lv_obj_t *scr = NULL;

static lv_obj_t *pd_v_label;
static lv_obj_t *v_val_label;
static lv_obj_t *i_val_label;
static lv_obj_t *w_val_label;
static lv_obj_t *v_const_cont;
static lv_obj_t *i_const_cont;

static void app_cb() {

    switch (esp_random() % 6) {
        case 0: lv_label_set_text(pd_v_label, "PD 5V"); break;
        case 1: lv_label_set_text(pd_v_label, "PD 9V"); break;
        case 2: lv_label_set_text(pd_v_label, "PD 12V"); break;
        case 3: lv_label_set_text(pd_v_label, "PD 15V"); break;
        case 4: lv_label_set_text(pd_v_label, "PD 18V"); break;
        case 5: lv_label_set_text(pd_v_label, "PD 20V"); break;
    }

    uint32_t mv = esp_random() % MILLI_V_MAX;
    uint32_t ma = esp_random() % MILLI_A_MAX;
    uint32_t mw = (mv * ma) / 1000;

    lv_label_set_text_fmt(v_val_label, "%d.%03d", (int)(mv/1000), (int)(mv%1000));
    lv_label_set_text_fmt(i_val_label, "%d.%03d", (int)(ma/1000), (int)(ma%1000));
    lv_label_set_text_fmt(w_val_label, "%d.%03d", (int)(mw/1000), (int)(mw%1000));

    if (esp_random() % 2) {
        lv_obj_add_flag(v_const_cont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(i_const_cont, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(i_const_cont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(v_const_cont, LV_OBJ_FLAG_HIDDEN);
    }
}

static lv_style_t style_base;
static lv_style_t style_focus;
static lv_style_t style_edit;
static lv_style_t style_font_small;
static lv_style_t style_font_mid;
static lv_style_t style_font_big;

static void create_common_styles() {
    lv_style_init(&style_base);
    lv_style_set_bg_color(&style_base, lv_color_hex(0x000000));
    lv_style_set_text_color(&style_base, lv_color_hex(0xffffff));
    lv_style_set_text_align(&style_base, LV_TEXT_ALIGN_CENTER);
    lv_style_set_pad_top(&style_base, 0);
    lv_style_set_pad_bottom(&style_base, 0);
    lv_style_set_pad_left(&style_base, 0);
    lv_style_set_pad_right(&style_base, 0);
    lv_style_set_pad_row(&style_base, 0);
    lv_style_set_pad_column(&style_base, 0);
    lv_style_set_outline_width(&style_base, 0);
    lv_style_set_border_width(&style_base, 0);
#if 0
    lv_style_set_border_width(&style_base, 1);
    lv_style_set_border_color(&style_base, lv_color_hex(0xff0000));
#endif

    lv_style_init(&style_focus);
    lv_style_set_outline_width(&style_focus, 0);
    lv_style_set_border_width(&style_focus, 2);
    lv_style_set_border_color(&style_focus, lv_color_hex(0x0000ff));

    lv_style_init(&style_edit);
    lv_style_set_outline_width(&style_edit, 0);
    lv_style_set_border_width(&style_edit, 2);
    lv_style_set_border_color(&style_edit, lv_color_hex(0xff0000));

    lv_style_init(&style_font_small);
    lv_style_set_text_font(&style_font_small, &lv_font_montserrat_20);

    lv_style_init(&style_font_mid);
    lv_style_set_text_font(&style_font_mid, &lv_font_montserrat_32);

    lv_style_init(&style_font_big);
    lv_style_set_text_font(&style_font_big, &lv_font_montserrat_48);
}

// Will be called when the styles of the base theme are already added to add new styles
static void theme_cb(lv_theme_t * th, lv_obj_t * obj) {
    (void)th;

    lv_obj_add_style(obj, &style_base, LV_STATE_DEFAULT);
    lv_obj_add_style(obj, &style_focus, LV_STATE_FOCUS_KEY);
    lv_obj_add_style(obj, &style_edit, LV_STATE_FOCUS_KEY | LV_STATE_EDITED);

//    if(lv_obj_check_type(obj, &lv_btn_class)) {
//        lv_obj_add_style(obj, &style_btn, 0);
//    }
}

static void create_theme() {
    // Initialize the new theme from the current theme
    lv_theme_t * th_act = lv_disp_get_theme(NULL);
    static lv_theme_t th_new;
    th_new = *th_act;

    lv_theme_set_parent(&th_new, th_act);
    lv_theme_set_apply_cb(&th_new, theme_cb);
    lv_disp_set_theme(NULL, &th_new);
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

    create_common_styles();
    create_theme();

    static lv_style_t style_if;
    lv_style_set_radius(&style_if, 0);
    lv_style_set_border_width(&style_if, 1);
    lv_style_set_border_color(&style_if, lv_color_hex(0xffffff));

    static lv_style_t style_cv;
    lv_style_set_bg_color(&style_cv, lv_color_hex(0xff0000));
    lv_style_set_text_color(&style_cv, lv_color_hex(0x000000));

    static lv_style_t style_cc;
    lv_style_set_bg_color(&style_cc, lv_color_hex(0x0000ff));
    lv_style_set_text_color(&style_cc, lv_color_hex(0x000000));

    scr = lv_disp_get_scr_act(NULL);

    // Interface bar
    lv_obj_t *if_grid = lv_obj_create(scr);

    lv_obj_t *btn_menu = lv_btn_create(if_grid);
    lv_obj_add_style(btn_menu, &style_font_small, LV_STATE_DEFAULT);
    lv_obj_add_style(btn_menu, &style_if, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(btn_menu, LV_SYMBOL_SETTINGS, 0);
    lv_obj_add_event_cb(btn_menu, display_indev_touch_btn_cb, LV_EVENT_ALL, (void*)DISPLAY_INDEV_TOUCH_BTN_MENU);

    lv_obj_t *btn_up = lv_btn_create(if_grid);
    lv_obj_add_style(btn_up, &style_font_small, LV_STATE_DEFAULT);
    lv_obj_add_style(btn_up, &style_if, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(btn_up, LV_SYMBOL_UP, 0);
    lv_obj_add_event_cb(btn_up, display_indev_touch_btn_cb, LV_EVENT_ALL, (void*)DISPLAY_INDEV_TOUCH_BTN_UP);

    lv_obj_t *btn_down = lv_btn_create(if_grid);
    lv_obj_add_style(btn_down, &style_font_small, LV_STATE_DEFAULT);
    lv_obj_add_style(btn_down, &style_if, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(btn_down, LV_SYMBOL_DOWN, 0);
    lv_obj_add_event_cb(btn_down, display_indev_touch_btn_cb, LV_EVENT_ALL, (void*)DISPLAY_INDEV_TOUCH_BTN_DOWN);

    lv_obj_t *btn_enter = lv_btn_create(if_grid);
    lv_obj_add_style(btn_enter, &style_font_small, LV_STATE_DEFAULT);
    lv_obj_add_style(btn_enter, &style_if, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(btn_enter, LV_SYMBOL_NEW_LINE, 0);
    lv_obj_add_event_cb(btn_enter, display_indev_touch_btn_cb, LV_EVENT_ALL, (void*)DISPLAY_INDEV_TOUCH_BTN_ENTER);

    // Placeholder
    lv_obj_t *ph_cont = lv_obj_create(if_grid);
    lv_obj_add_style(ph_cont, &style_if, LV_STATE_DEFAULT);
    lv_obj_t *ph_label = lv_label_create(ph_cont);
    lv_obj_add_style(ph_label, &style_font_small, LV_STATE_DEFAULT);
    lv_obj_set_align(ph_label, LV_ALIGN_CENTER);
    lv_label_set_text(ph_label, "");

    lv_obj_t *pd_v_cont = lv_obj_create(if_grid);
    lv_obj_add_style(pd_v_cont, &style_if, LV_STATE_DEFAULT);
    pd_v_label = lv_label_create(pd_v_cont);
    lv_obj_add_style(pd_v_label, &style_font_small, LV_STATE_DEFAULT);
    lv_obj_set_align(pd_v_label, LV_ALIGN_CENTER);
    lv_label_set_text(pd_v_label, "PD -V");

    // View screen
    lv_obj_t *view_grid = lv_obj_create(scr);

    // voltage fields
    lv_obj_t *v_grid = lv_obj_create(view_grid);
    lv_obj_set_height(v_grid, LV_SIZE_CONTENT);

    lv_obj_t *v_set_str_cont = lv_obj_create(v_grid);
    lv_obj_t *v_set_str_label = lv_label_create(v_set_str_cont);
    lv_obj_add_style(v_set_str_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(v_set_str_label, LV_ALIGN_CENTER);
    lv_label_set_text(v_set_str_label, "V set");

    lv_obj_t *v_set_cont = lv_obj_create(v_grid);
    lv_obj_t *v_set_spinbox = lv_spinbox_create(v_set_cont);
    lv_obj_add_style(v_set_spinbox, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(v_set_spinbox, LV_ALIGN_CENTER);
    lv_spinbox_set_range(v_set_spinbox, 0, 25000);
    lv_spinbox_set_digit_format(v_set_spinbox, 5, 2);
    lv_spinbox_set_cursor_pos(v_set_spinbox, 0);

    lv_obj_t *v_val_cont = lv_obj_create(v_grid);
    v_val_label = lv_label_create(v_val_cont);
    lv_obj_add_style(v_val_label, &style_font_big, LV_STATE_DEFAULT);
    lv_obj_set_align(v_val_label, LV_ALIGN_CENTER);
    lv_label_set_text(v_val_label, "0.000");

    v_const_cont = lv_obj_create(v_grid);
    lv_obj_add_style(v_const_cont, &style_cv, LV_STATE_DEFAULT);
    lv_obj_add_flag(v_const_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *v_const_label = lv_label_create(v_const_cont);
    lv_obj_add_style(v_const_label, &style_cv, LV_STATE_DEFAULT);
    lv_obj_add_style(v_const_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(v_const_label, LV_ALIGN_CENTER);
    lv_label_set_text(v_const_label, "CV");

    lv_obj_t *v_unit_cont = lv_obj_create(v_grid);
    lv_obj_t *v_unit_label = lv_label_create(v_unit_cont);
    lv_obj_add_style(v_unit_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(v_unit_label, LV_ALIGN_CENTER);
    lv_label_set_text(v_unit_label, "V");

    // Current fields
    lv_obj_t *i_grid = lv_obj_create(view_grid);
    lv_obj_set_height(i_grid, LV_SIZE_CONTENT);

    lv_obj_t *i_set_str_cont = lv_obj_create(i_grid);
    lv_obj_t *i_set_str_label = lv_label_create(i_set_str_cont);
    lv_obj_add_style(i_set_str_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(i_set_str_label, LV_ALIGN_CENTER);
    lv_label_set_text(i_set_str_label, "I set");

    lv_obj_t *i_set_cont = lv_obj_create(i_grid);
    lv_obj_t *i_set_spinbox = lv_spinbox_create(i_set_cont);
    lv_obj_add_style(i_set_spinbox, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(i_set_spinbox, LV_ALIGN_CENTER);
    lv_spinbox_set_range(i_set_spinbox, 0, 2000);
    lv_spinbox_set_digit_format(i_set_spinbox, 4, 1);
    lv_spinbox_set_cursor_pos(i_set_spinbox, 0);

    lv_obj_t *i_val_cont = lv_obj_create(i_grid);
    i_val_label = lv_label_create(i_val_cont);
    lv_obj_add_style(i_val_label, &style_font_big, LV_STATE_DEFAULT);
    lv_obj_set_align(i_val_label, LV_ALIGN_CENTER);
    lv_label_set_text(i_val_label, "0.000");

    i_const_cont = lv_obj_create(i_grid);
    lv_obj_add_style(i_const_cont, &style_cc, LV_STATE_DEFAULT);
    lv_obj_add_flag(i_const_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *i_const_label = lv_label_create(i_const_cont);
    lv_obj_add_style(i_const_label, &style_cc, LV_STATE_DEFAULT);
    lv_obj_add_style(i_const_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(i_const_label, LV_ALIGN_CENTER);
    lv_label_set_text(i_const_label, "CC");

    lv_obj_t *i_unit_cont = lv_obj_create(i_grid);
    lv_obj_t *i_unit_label = lv_label_create(i_unit_cont);
    lv_obj_add_style(i_unit_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(i_unit_label, LV_ALIGN_CENTER);
    lv_label_set_text(i_unit_label, "A");

    // Power fields
    lv_obj_t *w_grid = lv_obj_create(view_grid);
    lv_obj_set_height(w_grid, LV_SIZE_CONTENT);

    lv_obj_t *w_val_cont = lv_obj_create(w_grid);
    w_val_label = lv_label_create(w_val_cont);
    lv_obj_add_style(w_val_label, &style_font_big, LV_STATE_DEFAULT);
    lv_obj_set_align(w_val_label, LV_ALIGN_CENTER);
    lv_label_set_text(w_val_label, "0.000");

    lv_obj_t *w_unit_cont = lv_obj_create(w_grid);
    lv_obj_t *w_unit_label = lv_label_create(w_unit_cont);
    lv_obj_add_style(w_unit_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(w_unit_label, LV_ALIGN_CENTER);
    lv_label_set_text(w_unit_label, "W");

    // Top grid
    static lv_coord_t main_grid_col[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t main_grid_row[] = {LV_GRID_FR(1), 50, LV_GRID_TEMPLATE_LAST};

    // Interface grid
    static lv_coord_t if_grid_col[] = {60, 60, 60, 60, LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t if_grid_row[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

    // View grid
    static lv_coord_t view_grid_col[] = {15, LV_GRID_FR(1), 15, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t view_grid_row[] = {15, LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), 15, LV_GRID_TEMPLATE_LAST};

    // Grid for voltage, current and power
    static lv_coord_t param_grid_col[] = {150, LV_GRID_FR(1), 50, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t param_grid_row[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(scr, main_grid_col, main_grid_row);
    lv_obj_set_grid_cell(view_grid, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(if_grid, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_set_grid_dsc_array(if_grid, if_grid_col, if_grid_row);
    lv_obj_set_grid_cell(btn_menu,  LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(btn_up,    LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(btn_down,  LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(btn_enter, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(ph_cont,   LV_GRID_ALIGN_STRETCH, 4, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(pd_v_cont, LV_GRID_ALIGN_STRETCH, 5, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_set_grid_dsc_array(view_grid, view_grid_col, view_grid_row);
    lv_obj_set_grid_cell(v_grid, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_grid_cell(i_grid, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
    lv_obj_set_grid_cell(w_grid, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 3, 1);

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

    lv_obj_set_grid_dsc_array(w_grid, param_grid_col, param_grid_row);
    lv_obj_set_grid_cell(w_val_cont,     LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
    lv_obj_set_grid_cell(w_unit_cont,    LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_group_t *group = lv_group_create();
    lv_group_add_obj(group, btn_menu);
    lv_group_add_obj(group, i_set_spinbox);
    lv_group_add_obj(group, v_set_spinbox);
    lv_indev_set_group(indev_encoder, group);

    lv_timer_t *timer = lv_timer_create(app_cb, 500, NULL);

    xSemaphoreGive(lv_task_sema);
}
