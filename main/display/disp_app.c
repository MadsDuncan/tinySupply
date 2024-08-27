#include "esp_random.h"
#include <string.h>

#include "display.h"
#include "disp_internal.h"

window_t windows[] = {
    [WINDOW_NONE]    = { .name = "NA",      .obj = NULL, .create_func = NULL,                  .menu_dropdown = false},
    [WINDOW_MENU]    = { .name = "Menu",    .obj = NULL, .create_func = create_menu_window,    .menu_dropdown = false},
    [WINDOW_VIEW]    = { .name = "View",    .obj = NULL, .create_func = create_view_window,    .menu_dropdown = true},
    [WINDOW_GRAPH]   = { .name = "Graph",   .obj = NULL, .create_func = create_graph_window,   .menu_dropdown = true},
    [WINDOW_DATALOG] = { .name = "Datalog", .obj = NULL, .create_func = create_datalog_window, .menu_dropdown = true}
};

window_identifier_t active_window = WINDOW_NONE;
window_identifier_t selected_window = WINDOW_VIEW;

// Prototypes
static void btn_menu_cb(lv_event_t *event);

// Common styles
lv_style_t style_view_base;
lv_style_t style_menu_base;
lv_style_t style_focus;
lv_style_t style_edit;
lv_style_t style_font_small;
lv_style_t style_font_mid;
lv_style_t style_font_big;

lv_obj_t *scr = NULL;
lv_obj_t *view_pd_v_label;

// Group related
lv_group_t *group;
lv_obj_t *btn_menu;

static void app_cb() {
    switch (esp_random() % 6) {
        case 0: lv_label_set_text(view_pd_v_label, "PD 5V"); break;
        case 1: lv_label_set_text(view_pd_v_label, "PD 9V"); break;
        case 2: lv_label_set_text(view_pd_v_label, "PD 12V"); break;
        case 3: lv_label_set_text(view_pd_v_label, "PD 15V"); break;
        case 4: lv_label_set_text(view_pd_v_label, "PD 18V"); break;
        case 5: lv_label_set_text(view_pd_v_label, "PD 20V"); break;
    }
#if 1
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

    uint32_t p = (v * i) / 1000; // Power in mW
    bool const_i = (esp_random() % 2) ? true : false;

    switch (active_window) {
        case WINDOW_VIEW:
            update_view(v, i, p, const_i);
            break;
        case WINDOW_GRAPH:
            add_graph_point(v, i);
            break;
        default:
            break;
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
    lv_obj_add_style(obj, &style_edit, LV_STATE_EDITED);

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

void obj_select_cb(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t *sb = lv_event_get_target(event);

    if (code == LV_EVENT_PRESSING) {
        lv_group_focus_obj(sb);
        lv_group_set_editing(group, true);
    } else if (code == LV_EVENT_LEAVE) {
        lv_group_set_editing(group, false);
    }
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
    view_pd_v_label = lv_label_create(pd_v_cont);
    lv_obj_add_style(view_pd_v_label, &style_font_small, LV_STATE_DEFAULT);
    lv_obj_set_align(view_pd_v_label, LV_ALIGN_CENTER);
    lv_label_set_text(view_pd_v_label, "PD NA");

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

    // Create global group. Will be populated by active window.
    group = lv_group_create();

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

    lv_timer_t *timer = lv_timer_create(app_cb, SAMPLE_PERIOD, NULL);

    xSemaphoreGive(lv_task_sema);
}
