#include "disp_internal.h"
#include <string.h>

#define GRAPH_X_MAX 1000
#define GRAPH_X_CLEAR_STEP (GRAPH_X_MAX / 4)

static lv_obj_t *graph;
static lv_chart_series_t *graph_series_v;
static lv_chart_series_t *graph_series_i;
static bool graph_ready = false;

static void graph_draw_cb(lv_event_t * event)
{
    lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(event);

    if(!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL)) {
        return;
    }

    if(dsc->id == LV_CHART_AXIS_PRIMARY_Y && dsc->text) {
        lv_snprintf(dsc->text, dsc->text_length, "%dV", (int)(dsc->value/1000));
    }

    if(dsc->id == LV_CHART_AXIS_SECONDARY_Y && dsc->text) {
        uint32_t d0 = dsc->value/1000;
        uint32_t d1 = (dsc->value/100)%10;
        lv_snprintf(dsc->text, dsc->text_length, "%d.%dA", (int)d0, (int)d1);
    }

    if(dsc->id == LV_CHART_AXIS_PRIMARY_X && dsc->text) {
        lv_snprintf(dsc->text, dsc->text_length, "t%d", (int)dsc->value);
    }
}

lv_obj_t* create_graph_window() {
    graph_ready = false;

    lv_obj_t *window = lv_obj_create(scr);

    graph = lv_chart_create(window);
    lv_obj_set_style_outline_width(graph, 1, 0);
    lv_obj_set_style_outline_color(graph, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_radius(graph, 0, 0);

    lv_chart_set_point_count(graph, GRAPH_X_MAX);
    lv_chart_set_range(graph, LV_CHART_AXIS_PRIMARY_Y, 0, MILLI_V_MAX);
    lv_chart_set_range(graph, LV_CHART_AXIS_SECONDARY_Y, 0, MILLI_A_MAX);

    lv_chart_set_type(graph, LV_CHART_TYPE_LINE); // Points and lines
    lv_chart_set_div_line_count(graph, 6, 11);
    lv_obj_add_event_cb(graph, graph_draw_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    lv_chart_set_axis_tick(graph, LV_CHART_AXIS_PRIMARY_X, 10, 5, 10, 5, true, 40);
    lv_chart_set_axis_tick(graph, LV_CHART_AXIS_PRIMARY_Y, 6, 0, 6, 3, true, 40);
    lv_chart_set_axis_tick(graph, LV_CHART_AXIS_SECONDARY_Y, 4, 0, 4, 2, true, 40);

    graph_series_v = lv_chart_add_series(graph, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    graph_series_i = lv_chart_add_series(graph, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_SECONDARY_Y);

    graph_ready = true;

    static lv_coord_t graph_grid_col[] = {40, LV_GRID_FR(1), 40, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t graph_grid_row[] = {15, LV_GRID_FR(1), 30, LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(window, graph_grid_col, graph_grid_row);
    lv_obj_set_grid_cell(graph, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_group_remove_all_objs(group); // Clear objects from previous window
    lv_group_add_obj(group, btn_menu);
    lv_indev_set_group(indev_encoder, group);

    return window;
}

void add_graph_point(uint32_t v, uint32_t i) {
    static uint32_t current_point = 0;

    if (!graph_ready) {
        return;
    }

    if (current_point == (GRAPH_X_MAX)) {

        for (uint32_t i = 0; i < GRAPH_X_MAX; i++) {
            if (i < GRAPH_X_MAX - GRAPH_X_CLEAR_STEP) {
                graph_series_v->y_points[i] = graph_series_v->y_points[i + GRAPH_X_CLEAR_STEP];
                graph_series_i->y_points[i] = graph_series_i->y_points[i + GRAPH_X_CLEAR_STEP];
            } else {
                graph_series_v->y_points[i] = LV_CHART_POINT_NONE;
                graph_series_i->y_points[i] = LV_CHART_POINT_NONE;
            }
        }

        current_point -= GRAPH_X_CLEAR_STEP;
    }

    graph_series_v->y_points[current_point] = v;
    graph_series_i->y_points[current_point] = i;

    current_point++;

    lv_chart_refresh(graph);
}
