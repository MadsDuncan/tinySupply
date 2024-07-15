#ifndef DISPLAY_ENCODER_H
#define DISPLAY_ENCODER_H

#include "lvgl.h"
#include <stdint.h>

void display_drv_encoder_setup();
void display_drv_encoder_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

#endif
