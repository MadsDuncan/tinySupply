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

extern SemaphoreHandle_t lv_task_sema;
extern lv_indev_t *indev_touchpad;
extern lv_indev_t *indev_encoder;

void display_drv_encoder_setup();
void display_drv_encoder_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

#endif
