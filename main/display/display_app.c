#include "esp_random.h"
#include <string.h>

#include "display.h"
#include "display_internal.h"

#define MILLI_V_MAX 25000
#define MILLI_A_MAX 1500

typedef enum {
  WINDOW_NONE = 0,
  WINDOW_MENU = 1,
  WINDOW_VIEW = 2,
  WINDOW_GRAPH = 3,
  WINDOW_DATALOG = 4,
  WINDOW_NUM = 5
} window_identifier_t;

typedef lv_obj_t* (*create_func_t)();

typedef struct {
    char name[10];
    lv_obj_t *obj;
    create_func_t create_func;
    bool menu_dropdown;
    uint8_t menu_dropdown_index;
} window_t;

static lv_obj_t* create_menu_window();
static lv_obj_t* create_view_window();
static lv_obj_t* create_graph_window();
static lv_obj_t* create_datalog_window();

static window_identifier_t active_window = WINDOW_NONE;
static window_identifier_t selected_window = WINDOW_VIEW;

static window_t windows[] = {
    [WINDOW_NONE]    = { .name = "NA",      .obj = NULL, .create_func = NULL,                  .menu_dropdown = false},
    [WINDOW_MENU]    = { .name = "Menu",    .obj = NULL, .create_func = create_menu_window,    .menu_dropdown = false},
    [WINDOW_VIEW]    = { .name = "View",    .obj = NULL, .create_func = create_view_window,    .menu_dropdown = true},
    [WINDOW_GRAPH]   = { .name = "Graph",   .obj = NULL, .create_func = create_graph_window,   .menu_dropdown = true},
    [WINDOW_DATALOG] = { .name = "Datalog", .obj = NULL, .create_func = create_datalog_window, .menu_dropdown = true}
};

// Prototypes
static void btn_menu_cb(lv_event_t *event);

// Common styles
static lv_style_t style_view_base;
static lv_style_t style_menu_base;
static lv_style_t style_focus;
static lv_style_t style_edit;
static lv_style_t style_font_small;
static lv_style_t style_font_mid;
static lv_style_t style_font_big;

static lv_obj_t *scr = NULL;

// Objects updated from callback
static lv_obj_t *pd_v_label;
static lv_obj_t *v_val_label;
static lv_obj_t *i_val_label;
static lv_obj_t *w_val_label;
static lv_obj_t *v_const_cont;
static lv_obj_t *i_const_cont;

// Global objects used in encoder indev group
static lv_obj_t *btn_menu;

static void app_cb() {
    if (active_window != WINDOW_VIEW) {
        return;
    }

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

static void create_common_styles() {
    lv_style_init(&style_view_base);
    lv_style_set_bg_color(&style_view_base, lv_color_hex(0x000000));
    lv_style_set_text_color(&style_view_base, lv_color_hex(0xffffff));
    lv_style_set_text_align(&style_view_base, LV_TEXT_ALIGN_CENTER);
    lv_style_set_pad_top(&style_view_base, 0);
    lv_style_set_pad_bottom(&style_view_base, 0);
    lv_style_set_pad_left(&style_view_base, 0);
    lv_style_set_pad_right(&style_view_base, 0);
    lv_style_set_pad_row(&style_view_base, 0);
    lv_style_set_pad_column(&style_view_base, 0);
    lv_style_set_outline_width(&style_view_base, 0);
    lv_style_set_border_width(&style_view_base, 0);
#if 0
    lv_style_set_border_width(&style_view_base, 1);
    lv_style_set_border_color(&style_view_base, lv_color_hex(0xff0000));
#endif

    lv_style_init(&style_menu_base);
    lv_style_set_bg_color(&style_menu_base, lv_color_hex(0x000000));
    lv_style_set_text_color(&style_menu_base, lv_color_hex(0xffffff));

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

    lv_obj_add_style(obj, &style_focus, LV_STATE_FOCUS_KEY);
    lv_obj_add_style(obj, &style_edit, LV_STATE_FOCUS_KEY | LV_STATE_EDITED);

    if (active_window == WINDOW_MENU) {
        lv_obj_add_style(obj, &style_menu_base, LV_STATE_DEFAULT);
    } else {
        lv_obj_add_style(obj, &style_view_base, LV_STATE_DEFAULT);
    }
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

static lv_obj_t* create_interface_bar() {
    static lv_style_t style_if;
    lv_style_set_radius(&style_if, 0);
    lv_style_set_border_width(&style_if, 1);
    lv_style_set_border_color(&style_if, lv_color_hex(0xffffff));

    lv_obj_t *bar = lv_obj_create(scr);

    // Buttons
    btn_menu = lv_btn_create(bar);
    lv_obj_add_style(btn_menu, &style_font_small, LV_STATE_DEFAULT);
    lv_obj_add_style(btn_menu, &style_if, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(btn_menu, LV_SYMBOL_SETTINGS, 0);
    lv_obj_add_event_cb(btn_menu, btn_menu_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *btn_up = lv_btn_create(bar);
    lv_obj_add_style(btn_up, &style_font_small, LV_STATE_DEFAULT);
    lv_obj_add_style(btn_up, &style_if, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(btn_up, LV_SYMBOL_UP, 0);
    lv_obj_add_event_cb(btn_up, display_indev_touch_btn_cb, LV_EVENT_ALL, (void*)DISPLAY_INDEV_TOUCH_BTN_UP);

    lv_obj_t *btn_down = lv_btn_create(bar);
    lv_obj_add_style(btn_down, &style_font_small, LV_STATE_DEFAULT);
    lv_obj_add_style(btn_down, &style_if, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(btn_down, LV_SYMBOL_DOWN, 0);
    lv_obj_add_event_cb(btn_down, display_indev_touch_btn_cb, LV_EVENT_ALL, (void*)DISPLAY_INDEV_TOUCH_BTN_DOWN);

    lv_obj_t *btn_enter = lv_btn_create(bar);
    lv_obj_add_style(btn_enter, &style_font_small, LV_STATE_DEFAULT);
    lv_obj_add_style(btn_enter, &style_if, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(btn_enter, LV_SYMBOL_NEW_LINE, 0);
    lv_obj_add_event_cb(btn_enter, display_indev_touch_btn_cb, LV_EVENT_ALL, (void*)DISPLAY_INDEV_TOUCH_BTN_ENTER);

    // Placeholder
    lv_obj_t *ph_cont = lv_obj_create(bar);
    lv_obj_add_style(ph_cont, &style_if, LV_STATE_DEFAULT);
    lv_obj_t *ph_label = lv_label_create(ph_cont);
    lv_obj_add_style(ph_label, &style_font_small, LV_STATE_DEFAULT);
    lv_obj_set_align(ph_label, LV_ALIGN_CENTER);
    lv_label_set_text(ph_label, "");

    // Power delivery
    lv_obj_t *pd_v_cont = lv_obj_create(bar);
    lv_obj_add_style(pd_v_cont, &style_if, LV_STATE_DEFAULT);
    pd_v_label = lv_label_create(pd_v_cont);
    lv_obj_add_style(pd_v_label, &style_font_small, LV_STATE_DEFAULT);
    lv_obj_set_align(pd_v_label, LV_ALIGN_CENTER);
    lv_label_set_text(pd_v_label, "PD NA");

    // Grid setup
    static lv_coord_t if_grid_col[] = {60, 60, 60, 60, LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t if_grid_row[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(bar, if_grid_col, if_grid_row);
    lv_obj_set_grid_cell(btn_menu,  LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(btn_up,    LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(btn_down,  LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(btn_enter, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(ph_cont,   LV_GRID_ALIGN_STRETCH, 4, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(pd_v_cont, LV_GRID_ALIGN_STRETCH, 5, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    return bar;
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

static void menu_window_dropdown_cb(lv_event_t *event) {
    lv_obj_t * dd = lv_event_get_target(event);

    char buf[32];
    lv_dropdown_get_selected_str(dd, buf, sizeof(buf));

    for (uint8_t i = 0; i < WINDOW_NUM; i++) {
        if (strcmp(buf, windows[i].name) == 0) {
            selected_window = i;
        }
    }
}

static lv_obj_t* create_menu_window_dropdown(lv_obj_t *parent) {
    lv_obj_t *obj = create_menu_item_name(parent, NULL, "Window");
    lv_obj_t *dd = lv_dropdown_create(obj);
    lv_dropdown_clear_options(dd); // Clear default options

    uint8_t dd_index = 0;
    for (uint8_t i = 0; i < WINDOW_NUM; i++) {
        if (windows[i].menu_dropdown) {
            windows[i].menu_dropdown_index = dd_index;
            lv_dropdown_add_option(dd, windows[i].name, dd_index);
            dd_index++;
        }
    }
    lv_dropdown_set_selected(dd, windows[selected_window].menu_dropdown_index);
    lv_obj_add_event_cb(dd, menu_window_dropdown_cb, LV_EVENT_VALUE_CHANGED, NULL);

    return dd;
}

static lv_obj_t* create_menu_window() {
    lv_obj_t *menu = lv_menu_create(scr);
    lv_obj_t *main_page = lv_menu_page_create(menu, NULL);

    lv_obj_t *section = lv_menu_section_create(main_page);
    lv_obj_t *window_dropdown = create_menu_window_dropdown(section);

    lv_menu_set_page(menu, main_page);

    lv_group_t *group = lv_group_create();
    lv_group_add_obj(group, btn_menu);
    lv_group_add_obj(group, window_dropdown);
    lv_indev_set_group(indev_encoder, group);

    return menu;
}

static lv_obj_t* create_view_window() {
    static lv_style_t style_cv;
    lv_style_set_bg_color(&style_cv, lv_color_hex(0xff0000));
    lv_style_set_text_color(&style_cv, lv_color_hex(0x000000));

    static lv_style_t style_cc;
    lv_style_set_bg_color(&style_cc, lv_color_hex(0x0000ff));
    lv_style_set_text_color(&style_cc, lv_color_hex(0x000000));

    lv_obj_t *window = lv_obj_create(scr);

    // voltage fields
    lv_obj_t *v_view = lv_obj_create(window);
    lv_obj_set_height(v_view, LV_SIZE_CONTENT);

    lv_obj_t *v_set_str_cont = lv_obj_create(v_view);
    lv_obj_t *v_set_str_label = lv_label_create(v_set_str_cont);
    lv_obj_add_style(v_set_str_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(v_set_str_label, LV_ALIGN_CENTER);
    lv_label_set_text(v_set_str_label, "V set");

    lv_obj_t *v_set_cont = lv_obj_create(v_view);
    lv_obj_t *v_set_spinbox = lv_spinbox_create(v_set_cont);
    lv_obj_add_style(v_set_spinbox, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(v_set_spinbox, LV_ALIGN_CENTER);
    lv_spinbox_set_range(v_set_spinbox, 0, 25000);
    lv_spinbox_set_digit_format(v_set_spinbox, 5, 2);
    lv_spinbox_set_cursor_pos(v_set_spinbox, 0);

    lv_obj_t *v_val_cont = lv_obj_create(v_view);
    v_val_label = lv_label_create(v_val_cont);
    lv_obj_add_style(v_val_label, &style_font_big, LV_STATE_DEFAULT);
    lv_obj_set_align(v_val_label, LV_ALIGN_CENTER);
    lv_label_set_text(v_val_label, "NA");

    v_const_cont = lv_obj_create(v_view);
    lv_obj_add_style(v_const_cont, &style_cv, LV_STATE_DEFAULT);
    lv_obj_add_flag(v_const_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *v_const_label = lv_label_create(v_const_cont);
    lv_obj_add_style(v_const_label, &style_cv, LV_STATE_DEFAULT);
    lv_obj_add_style(v_const_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(v_const_label, LV_ALIGN_CENTER);
    lv_label_set_text(v_const_label, "CV");

    lv_obj_t *v_unit_cont = lv_obj_create(v_view);
    lv_obj_t *v_unit_label = lv_label_create(v_unit_cont);
    lv_obj_add_style(v_unit_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(v_unit_label, LV_ALIGN_CENTER);
    lv_label_set_text(v_unit_label, "V");

    // Current fields
    lv_obj_t *i_view = lv_obj_create(window);
    lv_obj_set_height(i_view, LV_SIZE_CONTENT);

    lv_obj_t *i_set_str_cont = lv_obj_create(i_view);
    lv_obj_t *i_set_str_label = lv_label_create(i_set_str_cont);
    lv_obj_add_style(i_set_str_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(i_set_str_label, LV_ALIGN_CENTER);
    lv_label_set_text(i_set_str_label, "I set");

    lv_obj_t *i_set_cont = lv_obj_create(i_view);
    lv_obj_t *i_set_spinbox = lv_spinbox_create(i_set_cont);
    lv_obj_add_style(i_set_spinbox, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(i_set_spinbox, LV_ALIGN_CENTER);
    lv_spinbox_set_range(i_set_spinbox, 0, 2000);
    lv_spinbox_set_digit_format(i_set_spinbox, 4, 1);
    lv_spinbox_set_cursor_pos(i_set_spinbox, 0);

    lv_obj_t *i_val_cont = lv_obj_create(i_view);
    i_val_label = lv_label_create(i_val_cont);
    lv_obj_add_style(i_val_label, &style_font_big, LV_STATE_DEFAULT);
    lv_obj_set_align(i_val_label, LV_ALIGN_CENTER);
    lv_label_set_text(i_val_label, "NA");

    i_const_cont = lv_obj_create(i_view);
    lv_obj_add_style(i_const_cont, &style_cc, LV_STATE_DEFAULT);
    lv_obj_add_flag(i_const_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *i_const_label = lv_label_create(i_const_cont);
    lv_obj_add_style(i_const_label, &style_cc, LV_STATE_DEFAULT);
    lv_obj_add_style(i_const_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(i_const_label, LV_ALIGN_CENTER);
    lv_label_set_text(i_const_label, "CC");

    lv_obj_t *i_unit_cont = lv_obj_create(i_view);
    lv_obj_t *i_unit_label = lv_label_create(i_unit_cont);
    lv_obj_add_style(i_unit_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(i_unit_label, LV_ALIGN_CENTER);
    lv_label_set_text(i_unit_label, "A");

    // Power fields
    lv_obj_t *w_view = lv_obj_create(window);
    lv_obj_set_height(w_view, LV_SIZE_CONTENT);

    lv_obj_t *w_val_cont = lv_obj_create(w_view);
    w_val_label = lv_label_create(w_val_cont);
    lv_obj_add_style(w_val_label, &style_font_big, LV_STATE_DEFAULT);
    lv_obj_set_align(w_val_label, LV_ALIGN_CENTER);
    lv_label_set_text(w_val_label, "NA");

    lv_obj_t *w_unit_cont = lv_obj_create(w_view);
    lv_obj_t *w_unit_label = lv_label_create(w_unit_cont);
    lv_obj_add_style(w_unit_label, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(w_unit_label, LV_ALIGN_CENTER);
    lv_label_set_text(w_unit_label, "W");

    // Grid setup
    static lv_coord_t view_grid_col[] = {15, LV_GRID_FR(1), 15, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t view_grid_row[] = {15, LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), 15, LV_GRID_TEMPLATE_LAST};

    static lv_coord_t param_grid_col[] = {150, LV_GRID_FR(1), 50, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t param_grid_row[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(window, view_grid_col, view_grid_row);
    lv_obj_set_grid_cell(v_view, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_grid_cell(i_view, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
    lv_obj_set_grid_cell(w_view, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 3, 1);

    lv_obj_set_grid_dsc_array(v_view, param_grid_col, param_grid_row);
    lv_obj_set_grid_cell(v_set_str_cont, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(v_set_cont,     LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_grid_cell(v_val_cont,     LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
    lv_obj_set_grid_cell(v_const_cont,   LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(v_unit_cont,    LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_set_grid_dsc_array(i_view, param_grid_col, param_grid_row);
    lv_obj_set_grid_cell(i_set_str_cont, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(i_set_cont,     LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_grid_cell(i_val_cont,     LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
    lv_obj_set_grid_cell(i_const_cont,   LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(i_unit_cont,    LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_set_grid_dsc_array(w_view, param_grid_col, param_grid_row);
    lv_obj_set_grid_cell(w_val_cont,     LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
    lv_obj_set_grid_cell(w_unit_cont,    LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_group_t *group = lv_group_create();
    lv_group_add_obj(group, btn_menu);
    lv_group_add_obj(group, v_set_spinbox);
    lv_group_add_obj(group, i_set_spinbox);
    lv_indev_set_group(indev_encoder, group);

    return window;
}

static lv_obj_t* create_graph_window() {
    lv_obj_t *window = lv_obj_create(scr);

    lv_obj_t *label =  lv_label_create(window);
    lv_label_set_text(label, "Graph");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_group_t *group = lv_group_create();
    lv_group_add_obj(group, btn_menu);
    lv_indev_set_group(indev_encoder, group);

    return window;
}

static lv_obj_t* create_datalog_window() {
    lv_obj_t *window = lv_obj_create(scr);

    lv_obj_t *label =  lv_label_create(window);
    lv_label_set_text(label, "Datalog");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_group_t *group = lv_group_create();
    lv_group_add_obj(group, btn_menu);
    lv_indev_set_group(indev_encoder, group);

    return window;
}

static void set_window(window_identifier_t window) {
    if (window == active_window) {
        return;
    }

    if (active_window != WINDOW_NONE) {
        lv_obj_del(windows[active_window].obj);
    }

    active_window = window;

    windows[window].obj = windows[window].create_func();
    lv_obj_set_grid_cell(windows[window].obj, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
}

static void btn_menu_cb(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);

    if (code == LV_EVENT_SHORT_CLICKED) {
        if (active_window == WINDOW_MENU) {
            set_window(selected_window);
        } else {
            set_window(WINDOW_MENU);
        }
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

    create_common_styles();
    create_theme();

    scr = lv_disp_get_scr_act(NULL);
    lv_obj_t *if_bar = create_interface_bar();

    // Top grid
    static lv_coord_t scr_grid_col[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t scr_grid_row[] = {LV_GRID_FR(1), 50, LV_GRID_TEMPLATE_LAST};

    // Set static part of screen
    lv_obj_set_grid_dsc_array(scr, scr_grid_col, scr_grid_row);
    lv_obj_set_grid_cell(if_bar, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    // Show view window initially
    set_window(WINDOW_VIEW);

    lv_timer_t *timer = lv_timer_create(app_cb, 500, NULL);

    xSemaphoreGive(lv_task_sema);
}
