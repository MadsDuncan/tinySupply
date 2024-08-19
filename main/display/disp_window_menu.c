#include "disp_internal.h"

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
    lv_obj_add_event_cb(dd, obj_select_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(dd, menu_window_dropdown_cb, LV_EVENT_VALUE_CHANGED, NULL);


    return dd;
}

lv_obj_t* create_menu_window() {
    lv_obj_t *menu = lv_menu_create(scr);
    lv_obj_t *main_page = lv_menu_page_create(menu, NULL);

    lv_obj_t *section = lv_menu_section_create(main_page);
    lv_obj_t *window_dropdown = create_menu_window_dropdown(section);

    lv_menu_set_page(menu, main_page);

    lv_group_remove_all_objs(group); // Clear objects from previous window
    lv_group_add_obj(group, btn_menu);
    lv_group_add_obj(group, window_dropdown);
    lv_indev_set_group(indev_encoder, group);

    return menu;
}
