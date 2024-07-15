#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define DISPLAY_DEMO_NONE        0
#define DISPLAY_DEMO_LVGL        1
#define DISPLAY_DEMO_HELLO_WORLD 2
#define DISPLAY_DEMO_FULL_SCREEN 3
#define DISPLAY_DEMO_THIN_RECT   4
#define DISPLAY_DEMO_APP         5
#define DISPLAY_DEMO_SPINBOX     6

void display_setup();
void display_start_demo(uint8_t demo);
void display_stop_demo();

#endif
