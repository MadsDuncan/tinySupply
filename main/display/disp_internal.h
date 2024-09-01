#ifndef DISPLAY_INTERNAL_H
#define DISPLAY_INTERNAL_H

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "sdkconfig.h"
#include <stdint.h>

#define TAG "display"

/************************************************
 *      Common
 ***********************************************/
#define V_MAX         25000 // Max voltage in mV
#define I_MAX         1500  // Max current in mA
#define SAMPLE_PERIOD 500   // Measurement sample rate in ms

extern SemaphoreHandle_t lv_task_sema;

extern lv_style_t style_view_base;
extern lv_style_t style_menu_base;
extern lv_style_t style_focus;
extern lv_style_t style_edit;
extern lv_style_t style_font_small;
extern lv_style_t style_font_mid;
extern lv_style_t style_font_big;

void obj_select_cb(lv_event_t *event);

/************************************************
 *      Input devices
 ***********************************************/
#define DISPLAY_INDEV_TOUCH_BTN_UP      1
#define DISPLAY_INDEV_TOUCH_BTN_DOWN    2
#define DISPLAY_INDEV_TOUCH_BTN_ENTER   3

extern lv_indev_t *indev_touchpad;
extern lv_indev_t *indev_encoder;

void display_indev_setup();
void display_indev_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
void display_indev_touch_btn_cb(lv_event_t *event);

/************************************************
 *      Display app
 ***********************************************/
typedef enum {
  WINDOW_NONE = 0,
  WINDOW_MENU = 1,
  WINDOW_VIEW = 2,
  WINDOW_GRAPH = 3,
  WINDOW_DATALOG = 4,
  WINDOW_NUM = 5
} window_identifier_t;

typedef lv_obj_t* (*create_func_t)();
typedef void (*remove_func_t)();

typedef struct {
    char name[10];
    lv_obj_t *obj;
    create_func_t create_func;
    remove_func_t remove_func;
    bool menu_dropdown;
    uint8_t menu_dropdown_index;
} window_t;

extern window_t windows[];
extern window_identifier_t active_window;
extern window_identifier_t selected_window;

extern lv_obj_t *scr;
extern lv_group_t *group;
extern lv_obj_t *btn_menu;

/************************************************
 *      Menu window
 ***********************************************/
lv_obj_t* create_menu_window();

/************************************************
 *      View window
 ***********************************************/
lv_obj_t* create_view_window();
void update_view(uint32_t v, uint32_t i, uint32_t p, bool const_i);

/************************************************
 *      Graph window
 ***********************************************/
lv_obj_t* create_graph_window();
void remove_graph_window();
void clear_graph();
void add_graph_point(uint32_t v, uint32_t i);

/************************************************
 *      Datalogger window
 ***********************************************/
lv_obj_t* create_datalog_window();

#endif
