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

#define DISPLAY_INDEV_TOUCH_BTN_UP      1
#define DISPLAY_INDEV_TOUCH_BTN_DOWN    2
#define DISPLAY_INDEV_TOUCH_BTN_ENTER   3

extern SemaphoreHandle_t lv_task_sema;
extern lv_indev_t *indev_touchpad;
extern lv_indev_t *indev_encoder;

void display_indev_setup();
void display_indev_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
void display_indev_touch_btn_cb(lv_event_t *event);

#endif
