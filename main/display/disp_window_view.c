#include "disp_internal.h"

lv_obj_t *view_pd_v_label;
lv_obj_t *view_v_val_label;
lv_obj_t *view_i_val_label;
lv_obj_t *view_w_val_label;
lv_obj_t *view_v_const_cont;
lv_obj_t *view_i_const_cont;

lv_obj_t* create_view_window() {
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
    lv_obj_add_event_cb(v_set_spinbox, obj_select_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_style(v_set_spinbox, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(v_set_spinbox, LV_ALIGN_CENTER);
    lv_spinbox_set_range(v_set_spinbox, 0, 25000);
    lv_spinbox_set_digit_format(v_set_spinbox, 5, 2);
    lv_spinbox_set_cursor_pos(v_set_spinbox, 0);

    lv_obj_t *v_val_cont = lv_obj_create(v_view);
    view_v_val_label = lv_label_create(v_val_cont);
    lv_obj_add_style(view_v_val_label, &style_font_big, LV_STATE_DEFAULT);
    lv_obj_set_align(view_v_val_label, LV_ALIGN_CENTER);
    lv_label_set_text(view_v_val_label, "NA");

    view_v_const_cont = lv_obj_create(v_view);
    lv_obj_add_style(view_v_const_cont, &style_cv, LV_STATE_DEFAULT);
    lv_obj_add_flag(view_v_const_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *v_const_label = lv_label_create(view_v_const_cont);
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
    lv_obj_add_event_cb(i_set_spinbox, obj_select_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_style(i_set_spinbox, &style_font_mid, LV_STATE_DEFAULT);
    lv_obj_set_align(i_set_spinbox, LV_ALIGN_CENTER);
    lv_spinbox_set_range(i_set_spinbox, 0, 2000);
    lv_spinbox_set_digit_format(i_set_spinbox, 4, 1);
    lv_spinbox_set_cursor_pos(i_set_spinbox, 0);

    lv_obj_t *i_val_cont = lv_obj_create(i_view);
    view_i_val_label = lv_label_create(i_val_cont);
    lv_obj_add_style(view_i_val_label, &style_font_big, LV_STATE_DEFAULT);
    lv_obj_set_align(view_i_val_label, LV_ALIGN_CENTER);
    lv_label_set_text(view_i_val_label, "NA");

    view_i_const_cont = lv_obj_create(i_view);
    lv_obj_add_style(view_i_const_cont, &style_cc, LV_STATE_DEFAULT);
    lv_obj_add_flag(view_i_const_cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *i_const_label = lv_label_create(view_i_const_cont);
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
    view_w_val_label = lv_label_create(w_val_cont);
    lv_obj_add_style(view_w_val_label, &style_font_big, LV_STATE_DEFAULT);
    lv_obj_set_align(view_w_val_label, LV_ALIGN_CENTER);
    lv_label_set_text(view_w_val_label, "NA");

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
    lv_obj_set_grid_cell(view_v_const_cont,   LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(v_unit_cont,    LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_set_grid_dsc_array(i_view, param_grid_col, param_grid_row);
    lv_obj_set_grid_cell(i_set_str_cont, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(i_set_cont,     LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_grid_cell(i_val_cont,     LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
    lv_obj_set_grid_cell(view_i_const_cont,   LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_grid_cell(i_unit_cont,    LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_set_grid_dsc_array(w_view, param_grid_col, param_grid_row);
    lv_obj_set_grid_cell(w_val_cont,     LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 2);
    lv_obj_set_grid_cell(w_unit_cont,    LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_group_remove_all_objs(group); // Clear objects from previous window
    lv_group_add_obj(group, btn_menu);
    lv_group_add_obj(group, v_set_spinbox);
    lv_group_add_obj(group, i_set_spinbox);
    lv_indev_set_group(indev_encoder, group);

    return window;
}
