#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

#define DISPLAY_UPDATE_PERIOD 500

#define DISPLAY_DEMO_NONE        0
#define DISPLAY_DEMO_LVGL        1
#define DISPLAY_DEMO_HELLO_WORLD 2
#define DISPLAY_DEMO_FULL_SCREEN 3
#define DISPLAY_DEMO_THIN_RECT   4
#define DISPLAY_DEMO_APP         5
#define DISPLAY_DEMO_SPINBOX     6
#define DISPLAY_DEMO_MENU        7

void display_setup();
void display_start_app();
void display_update(uint32_t v, uint32_t i, bool const_i, uint8_t pd_v);
void display_start_demo(uint8_t demo);
void display_stop_demo();

#endif
