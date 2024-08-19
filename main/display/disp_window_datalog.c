#include "disp_internal.h"

lv_obj_t* create_datalog_window() {
    lv_obj_t *window = lv_obj_create(scr);

    lv_obj_t *label =  lv_label_create(window);
    lv_label_set_text(label, "Datalog");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_group_remove_all_objs(group); // Clear objects from previous window
    lv_group_add_obj(group, btn_menu);
    lv_indev_set_group(indev_encoder, group);

    return window;
}
