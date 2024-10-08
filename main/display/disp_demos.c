#include "lvgl.h"
#include "demos/lv_demos.h"

#include "display.h"
#include "disp_internal.h"

static uint8_t active_demo = DISPLAY_DEMO_NONE;

static lv_style_t style0, style1;
static lv_obj_t *demo_scr = NULL;
static lv_obj_t *obj = NULL;
static lv_timer_t *timer = NULL;

/************************************************
 *      Simple demos
 ***********************************************/
static void hello_world_demo() {
    demo_scr = lv_disp_get_scr_act(NULL);
    obj =  lv_label_create(demo_scr);
    lv_label_set_text(obj, "Hello\nworld");
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
}

static void full_screen_demo_cb() {
    static bool switch_color = false;
    switch_color = !switch_color;

    if (switch_color) {
        lv_obj_remove_style(demo_scr, &style0, LV_PART_ANY | LV_STATE_ANY);
        lv_obj_add_style(demo_scr, &style1, 0);
    } else {
        lv_obj_remove_style(demo_scr, &style1, LV_PART_ANY | LV_STATE_ANY);
        lv_obj_add_style(demo_scr, &style0, 0);
    }
}

static void full_screen_demo() {
    lv_style_init(&style0);
    lv_style_set_bg_color(&style0, COLOR_WHITE);
    lv_style_set_text_color(&style0, COLOR_BLACK);

    lv_style_init(&style1);
    lv_style_set_bg_color(&style1, COLOR_BLACK);
    lv_style_set_text_color(&style1, COLOR_WHITE);

    demo_scr = lv_disp_get_scr_act(NULL);
    obj =  lv_label_create(demo_scr);
    lv_label_set_text(obj, "Hello\nworld");
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_style(demo_scr, &style0, 0);

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
    lv_style_set_bg_color(&style0, COLOR_GREEN);

    lv_style_init(&style1);
    lv_style_set_bg_color(&style1, COLOR_BLUE);

    demo_scr = lv_disp_get_scr_act(NULL);
    obj = lv_obj_create(demo_scr);
    lv_obj_set_size(obj , 50, 320);
    lv_obj_set_pos(obj , 0, 0);

    lv_obj_add_style(obj, &style0, 0);

    timer = lv_timer_create(thin_rect_demo_cb, 2000, NULL);
}

/************************************************
 *      Application view demo
 ***********************************************/
#include "esp_random.h"

lv_obj_t *demo_v_val_label;
lv_obj_t *demo_i_val_label;
lv_obj_t *demo_v_const_cont;
lv_obj_t *demo_i_const_cont;

static void app_demo_cb() {
    uint32_t mv = esp_random() % 25000;
    uint32_t ma = esp_random() % 1500;

    lv_label_set_text_fmt(demo_v_val_label, "%d.%03d", (int)(mv/1000), (int)(mv%1000));
    lv_label_set_text_fmt(demo_i_val_label, "%d.%03d", (int)(ma/1000), (int)(ma%1000));

    if (esp_random() % 2) {
        lv_obj_add_flag(demo_v_const_cont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(demo_i_const_cont, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(demo_i_const_cont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(demo_v_const_cont, LV_OBJ_FLAG_HIDDEN);
    }
}

static void app_demo() {
    static lv_style_t style_base;
    lv_style_init(&style_base);
    lv_style_set_bg_color(&style_base, COLOR_BLACK);
    lv_style_set_text_color(&style_base, COLOR_WHITE);
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
    lv_style_set_border_color(&style_base, COLOR_RED);
#endif

    static lv_style_t style_font_small;
    lv_style_set_text_font(&style_font_small, &lv_font_montserrat_20);

    static lv_style_t style_font_mid;
    lv_style_set_text_font(&style_font_mid, &lv_font_montserrat_32);

    static lv_style_t style_font_big;
    lv_style_set_text_font(&style_font_big, &lv_font_montserrat_48);

    static lv_style_t style_cv;
    lv_style_set_bg_color(&style_cv, COLOR_RED);
    lv_style_set_text_color(&style_cv, COLOR_BLACK);

    static lv_style_t style_cc;
    lv_style_set_bg_color(&style_cc, COLOR_BLUE);
    lv_style_set_text_color(&style_cc, COLOR_BLACK);

    demo_scr = lv_disp_get_scr_act(NULL);
    lv_obj_add_style(demo_scr, &style_base, 0);

    // voltage fields
    lv_obj_t *v_grid = lv_obj_create(demo_scr);
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
    demo_v_val_label = lv_label_create(v_val_cont);
    lv_obj_add_style(demo_v_val_label, &style_base, 0);
    lv_obj_add_style(demo_v_val_label, &style_font_big, 0);
    lv_obj_set_align(demo_v_val_label, LV_ALIGN_CENTER);
    lv_label_set_text(demo_v_val_label, "12.232");

    demo_v_const_cont = lv_obj_create(v_grid);
    lv_obj_add_style(demo_v_const_cont, &style_base, 0);
    lv_obj_add_style(demo_v_const_cont, &style_cv, 0);
    lv_obj_add_flag(demo_v_const_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *v_const_label = lv_label_create(demo_v_const_cont);
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
    lv_obj_t *i_grid = lv_obj_create(demo_scr);
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
    demo_i_val_label = lv_label_create(i_val_cont);
    lv_obj_add_style(demo_i_val_label, &style_base, 0);
    lv_obj_add_style(demo_i_val_label, &style_font_big, 0);
    lv_obj_set_align(demo_i_val_label, LV_ALIGN_CENTER);
    lv_label_set_text(demo_i_val_label, "0.990");

    demo_i_const_cont = lv_obj_create(i_grid);
    lv_obj_add_style(demo_i_const_cont, &style_base, 0);
    lv_obj_add_style(demo_i_const_cont, &style_cc, 0);
    lv_obj_add_flag(demo_i_const_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *i_const_label = lv_label_create(demo_i_const_cont);
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


    lv_obj_set_grid_dsc_array(demo_scr, main_grid_col, main_grid_row);
    lv_obj_set_grid_cell(v_grid, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_grid_cell(i_grid, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);

    lv_obj_set_grid_dsc_array(v_grid, param_grid_col, param_grid_row);
    lv_obj_set_grid_cell(v_set_str_cont,    LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(v_set_cont,        LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_grid_cell(v_val_cont,        LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
    lv_obj_set_grid_cell(demo_v_const_cont, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(v_unit_cont,       LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_set_grid_dsc_array(i_grid, param_grid_col, param_grid_row);
    lv_obj_set_grid_cell(i_set_str_cont,    LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(i_set_cont,        LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_grid_cell(i_val_cont,        LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
    lv_obj_set_grid_cell(demo_i_const_cont, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(i_unit_cont,       LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    timer = lv_timer_create(app_demo_cb, 500, NULL);
}

/************************************************
 *      Spinbox demo
 ***********************************************/
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

/************************************************
 *      Menu demo
 ***********************************************/
static void menu_demo_back_btn_cb(lv_event_t *event) {
    lv_obj_t * obj = lv_event_get_target(event);
    lv_obj_t * menu = lv_event_get_user_data(event);

    if(lv_menu_back_btn_is_root(menu, obj)) {
        lv_obj_t * mbox1 = lv_msgbox_create(NULL, "Hello", "Root back btn click.", NULL, true);
        lv_obj_center(mbox1);
    }
}

static lv_obj_t* create_menu_item_name(lv_obj_t *parent, const char *icon, const char *name) {
    lv_obj_t *obj = lv_menu_cont_create(parent);

    lv_obj_t *img = NULL;
    if(icon) {
        img = lv_img_create(obj);
        lv_img_set_src(img, icon);
    }

    lv_obj_t *label = lv_label_create(obj);
    lv_label_set_text(label, name);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_flex_grow(label, 1);

    return obj;
}

static lv_obj_t* create_menu_item_switch(lv_obj_t *parent, const char *icon, const char *name, bool checked) {
    lv_obj_t *obj = create_menu_item_name(parent, icon, name);

    lv_obj_t *sw = lv_switch_create(obj);
    lv_obj_add_state(sw, checked ? LV_STATE_CHECKED : 0);

    return sw;
}

static lv_obj_t* create_menu_item_dropdown(lv_obj_t *parent, const char *icon, const char *name, char *options) {
    lv_obj_t *obj = create_menu_item_name(parent, icon, name);

    lv_obj_t *dd = lv_dropdown_create(obj);
    lv_dropdown_set_options(dd, options);

    return dd;
}

static lv_obj_t* create_menu_item_info(lv_obj_t *parent, const char *icon, const char *name, char *info) {
    lv_obj_t *obj = create_menu_item_name(parent, icon, name);

    lv_obj_t *label = lv_label_create(obj);
    lv_label_set_text(label, info);

    return label;
}

static void inc_sb_cb(lv_event_t *event) {
    lv_obj_t *sb = (lv_obj_t*)event->user_data;
    lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_SHORT_CLICKED || code  == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(sb);
    }
}

static void dec_sb_cb(lv_event_t *event) {
    lv_obj_t *sb = (lv_obj_t*)event->user_data;
    lv_event_code_t code = lv_event_get_code(event);

    if(code == LV_EVENT_SHORT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(sb);
    }
}

static lv_obj_t* create_menu_item_spinbox(lv_obj_t *parent, const char *icon, const char *name) {
    lv_obj_t *obj = create_menu_item_name(parent, icon, name);

    lv_obj_t *sb = lv_spinbox_create(obj);
    lv_spinbox_set_digit_format(sb, 2, 0);
    lv_spinbox_set_range(sb, 0, 10);

    lv_coord_t h = lv_obj_get_height(sb);

    lv_obj_t *btn_p = lv_btn_create(obj);
    lv_obj_set_style_bg_img_src(btn_p, LV_SYMBOL_PLUS, 0);
    lv_obj_set_size(btn_p, h, h);
    lv_obj_add_event_cb(btn_p, inc_sb_cb, LV_EVENT_ALL, (void*)sb);

    lv_obj_t *btn_m = lv_btn_create(obj);
    lv_obj_set_style_bg_img_src(btn_m, LV_SYMBOL_MINUS, 0);
    lv_obj_set_size(btn_m, h, h);
    lv_obj_add_event_cb(btn_m, dec_sb_cb, LV_EVENT_ALL, (void*)sb);

    return sb;
}

void menu_demo() {
    /*Create a menu object*/
    lv_obj_t *menu = lv_menu_create(lv_scr_act());
    lv_obj_set_size(menu, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_center(menu);

    /*Modify the header*/
    lv_obj_t *back_btn = lv_menu_get_main_header_back_btn(menu);
    lv_obj_t *back_btn_label = lv_label_create(back_btn);
    lv_label_set_text(back_btn_label, "Back");
    lv_menu_set_mode_root_back_btn(menu, LV_MENU_ROOT_BACK_BTN_ENABLED);
    lv_obj_add_event_cb(menu, menu_demo_back_btn_cb, LV_EVENT_CLICKED, menu);


    lv_obj_t *cont;
    lv_obj_t *label;
    lv_obj_t *section;

    // Submenu
    lv_obj_t *subpage = lv_menu_page_create(menu, "Submenu");
    cont = lv_menu_cont_create(subpage);
    label = lv_label_create(cont);
    lv_label_set_text(label, "Hello, I am hiding here");

    // Main menu
    lv_obj_t *main_page = lv_menu_page_create(menu, "Menu");

    lv_obj_t *submenu_cont = lv_menu_cont_create(main_page);
    label = lv_label_create(submenu_cont);
    lv_label_set_text(label, "Subpage");
    lv_menu_set_load_page_event(menu, submenu_cont, subpage);

    section = lv_menu_section_create(main_page);
    lv_obj_t *switch1 = create_menu_item_switch(section, NULL, "Switch", false);

    section = lv_menu_section_create(main_page);
    lv_obj_t *dropdown = create_menu_item_dropdown(section, NULL, "Drop down", "A\n" "B\n" "C");

    section = lv_menu_section_create(main_page);
    lv_obj_t *info1 = create_menu_item_info(section, NULL, "Text 1", "Interesting stuff here!");

    section = lv_menu_section_create(main_page);
    lv_obj_t *info2 = create_menu_item_info(section, NULL, "Text 2", "Multi\n" "line\n" "stuff\n" "here");

    section = lv_menu_section_create(main_page);
    lv_obj_t *spinbox1 = create_menu_item_spinbox(section, NULL, "Spinbox 1");

    section = lv_menu_section_create(main_page);
    lv_obj_t *spinbox2 = create_menu_item_spinbox(section, NULL, "Spinbox 2");

    lv_menu_set_page(menu, main_page);

    lv_group_t *group = lv_group_create();
    lv_group_add_obj(group, submenu_cont);
    lv_group_add_obj(group, back_btn);
    lv_group_add_obj(group, switch1);
    lv_group_add_obj(group, dropdown);
    lv_group_add_obj(group, spinbox1);
    lv_group_add_obj(group, spinbox2);
    lv_indev_set_group(indev_encoder, group);
}

/************************************************
 *      Start/stop functions
 ***********************************************/
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
        case DISPLAY_DEMO_MENU:
            menu_demo();
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
