#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

#define ENCODER_0 0
#define ENCODER_1 1

#define ENCODER_BTN_CLEAR 0
#define ENCODER_BTN_SET   1

#define ENCODER_DIR_NONE  0
#define ENCODER_DIR_LEFT  1
#define ENCODER_DIR_RIGHT 2

void encoder_setup();
uint32_t encoder_get_btn(uint32_t encoder);
uint32_t encoder_get_btn_sticky(uint32_t encoder);
uint32_t encoder_get_dir_sticky(uint32_t encoder);
void encoder_register_var(uint32_t encoder, uint32_t *var, uint32_t step_size, uint32_t low_limit, uint32_t high_limit);
void encoder_unregister_var(uint32_t encoder);

#endif
