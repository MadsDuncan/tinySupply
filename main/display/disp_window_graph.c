#include "disp_internal.h"
#include <string.h>

#define GRAPH_X_DIVISIONS       11
#define GRAPH_Y_V_DIVISIONS     6
#define GRAPH_Y_I_DIVISIONS     4

#define GRAPH_X_POINTS          1000
#define GRAPH_X_DIVISION_POINTS (GRAPH_X_POINTS / (GRAPH_X_DIVISIONS - 1)) // Points between divisions in ms
#define GRAPH_X_DIVISION_TIME   (GRAPH_X_DIVISION_POINTS * SAMPLE_PERIOD)  // Time between divisions in ms
#define GRAPH_X_CLEAR_DIVISIONS 4
#define GRAPH_X_CLEAR_POINTS    (GRAPH_X_CLEAR_DIVISIONS * GRAPH_X_DIVISION_POINTS)
#define GRAPH_X_CLEAR_TIME      (GRAPH_X_CLEAR_DIVISIONS * GRAPH_X_DIVISION_TIME)

#define MS_PER_DS 100
#define MS_PER_S  1000
#define MS_PER_M  (MS_PER_S * 60)
#define MS_PER_H  (MS_PER_M * 60)

static lv_obj_t *graph;
static lv_chart_series_t *series_v;
static lv_chart_series_t *series_i;
static lv_coord_t series_v_points[GRAPH_X_POINTS] = {[0 ... (GRAPH_X_POINTS)-1] = LV_CHART_POINT_NONE};
static lv_coord_t series_i_points[GRAPH_X_POINTS] = {[0 ... (GRAPH_X_POINTS)-1] = LV_CHART_POINT_NONE};
static bool window_ready = false;
static uint32_t current_point = 0;
static uint32_t graph_start_time = 0;

static void graph_draw_cb(lv_event_t * event) {
    lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(event);

    if(!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL)) {
        return;
    }

    if(dsc->id == LV_CHART_AXIS_PRIMARY_Y && dsc->text) {
        dsc->label_dsc->color = lv_color_hex(0xffffff);

        lv_snprintf(dsc->text, dsc->text_length, "%dV", (int)(dsc->value/1000));
    }

    if(dsc->id == LV_CHART_AXIS_SECONDARY_Y && dsc->text) {
        dsc->label_dsc->color = lv_color_hex(0xffffff);

        uint32_t d0 = dsc->value/1000;
        uint32_t d1 = (dsc->value/100)%10;
        lv_snprintf(dsc->text, dsc->text_length, "%d.%dA", (int)d0, (int)d1);
    }

    if(dsc->id == LV_CHART_AXIS_PRIMARY_X && dsc->text) {
        dsc->label_dsc->color = lv_color_hex(0xffffff);

        // Offset X labels so they fit next to each other
        if (dsc->value % 2) {
            dsc->label_dsc->ofs_y = 7;
        } else {
            dsc->label_dsc->ofs_y = -7;
        }

        uint32_t t = graph_start_time + (dsc->value * GRAPH_X_DIVISION_TIME);

        uint32_t h = t / MS_PER_H;
        uint32_t m = (t % MS_PER_H) / MS_PER_M;
        uint32_t s = (t % MS_PER_M) / MS_PER_S;
        uint32_t ds = (t % MS_PER_S) / MS_PER_DS;

#if 1 // All non-0 fields for each label
        char label_str[20] = "";
        char tmp_str[10] = "";

        if (h) {
            sprintf(tmp_str, "%dh", (int)h);
            strcat(label_str, tmp_str);
        }

        if (m) {
            sprintf(tmp_str, "%dm", (int)m);
            strcat(label_str, tmp_str);
        }

        if (ds) {
            sprintf(tmp_str, "%d.%ds", (int)s, (int)ds);
        } else {
            sprintf(tmp_str, "%ds", (int)s);
        }
        strcat(label_str, tmp_str);

        lv_snprintf(dsc->text, dsc->text_length, "%s", label_str);

#else // 2 most significant fields for each label
        if (h) {
            if (m) {
                lv_snprintf(dsc->text, dsc->text_length, "%dh%dm", (int)h, (int)m);
            } else {
                lv_snprintf(dsc->text, dsc->text_length, "%dh", (int)h);
            }
        } else if (m) {
            if (s) {
                lv_snprintf(dsc->text, dsc->text_length, "%dm%ds", (int)m, (int)s);
            } else {
                lv_snprintf(dsc->text, dsc->text_length, "%dm", (int)m);
            }
        } else {
            if (ds) {
                lv_snprintf(dsc->text, dsc->text_length, "%d.%ds", (int)s, (int)ds);
            } else {
                lv_snprintf(dsc->text, dsc->text_length, "%ds", (int)s);
            }
        }
#endif
    }
}

lv_obj_t* create_graph_window() {
    lv_obj_t *window = lv_obj_create(scr);

    graph = lv_chart_create(window);
    lv_obj_set_style_outline_width(graph, 1, 0);
    lv_obj_set_style_outline_color(graph, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_radius(graph, 0, 0);

    lv_chart_set_point_count(graph, GRAPH_X_POINTS);
    lv_chart_set_range(graph, LV_CHART_AXIS_PRIMARY_Y, 0, V_MAX);
    lv_chart_set_range(graph, LV_CHART_AXIS_SECONDARY_Y, 0, I_MAX);

    lv_chart_set_type(graph, LV_CHART_TYPE_LINE); // Points and lines
    lv_chart_set_div_line_count(graph, GRAPH_Y_V_DIVISIONS, GRAPH_X_DIVISIONS);
    lv_obj_add_event_cb(graph, graph_draw_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    lv_chart_set_axis_tick(graph, LV_CHART_AXIS_PRIMARY_X, GRAPH_X_DIVISIONS, 0, GRAPH_X_DIVISIONS, 5, true, 40);
    lv_chart_set_axis_tick(graph, LV_CHART_AXIS_PRIMARY_Y, GRAPH_Y_V_DIVISIONS, 0, GRAPH_Y_V_DIVISIONS, 3, true, 40);
    lv_chart_set_axis_tick(graph, LV_CHART_AXIS_SECONDARY_Y, GRAPH_Y_I_DIVISIONS, 0, GRAPH_Y_I_DIVISIONS, 2, true, 40);

    series_v = lv_chart_add_series(graph, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    series_i = lv_chart_add_series(graph, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_SECONDARY_Y);
    lv_chart_set_ext_y_array(graph, series_v, series_v_points);
    lv_chart_set_ext_y_array(graph, series_i, series_i_points);

    static lv_coord_t graph_grid_col[] = {40, LV_GRID_FR(1), 40, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t graph_grid_row[] = {15, LV_GRID_FR(1), 45, LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(window, graph_grid_col, graph_grid_row);
    lv_obj_set_grid_cell(graph, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_group_remove_all_objs(group); // Clear objects from previous window
    lv_group_add_obj(group, btn_menu);
    lv_indev_set_group(indev_encoder, group);

    window_ready = true;
    return window;
}

void remove_graph_window() {
    window_ready = false;
}

void clear_graph() {
    for (uint32_t i = 0; i < GRAPH_X_POINTS; i++) {
        series_v_points[i] = LV_CHART_POINT_NONE;
        series_i_points[i] = LV_CHART_POINT_NONE;
    }

    current_point = 0;
    graph_start_time = 0;
}

void add_graph_point(uint32_t v, uint32_t i) {
    if (current_point == GRAPH_X_POINTS) {

        for (uint32_t i = 0; i < GRAPH_X_POINTS; i++) {
            if (i < GRAPH_X_POINTS - (GRAPH_X_CLEAR_POINTS)) {
                series_v_points[i] = series_v_points[i + (GRAPH_X_CLEAR_POINTS)];
                series_i_points[i] = series_i_points[i + (GRAPH_X_CLEAR_POINTS)];
            } else {
                series_v_points[i] = LV_CHART_POINT_NONE;
                series_i_points[i] = LV_CHART_POINT_NONE;
            }
        }

        current_point -= GRAPH_X_CLEAR_POINTS;
        graph_start_time += GRAPH_X_CLEAR_TIME;
    }

    series_v_points[current_point] = v;
    series_i_points[current_point] = i;

    current_point++;

    if (window_ready) {
        lv_chart_refresh(graph);
    }
}
