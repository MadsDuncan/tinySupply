#include "driver/gpio.h"
#include "esp_attr.h"
#include "gpio_map.h"
#include <stdio.h>

#include "encoder.h"

typedef struct {
    uint32_t gpio_a;
    uint32_t gpio_b;
    uint32_t gpio_btn;
    uint32_t btn_sticky;
    uint32_t dir_sticky;
    uint32_t *var;
    uint32_t step_size;
    uint32_t low_limit;
    uint32_t high_limit;
} encoder_t;

encoder_t encoder_0 = {
    .gpio_a = GPIO_ENCODER_0_A,
    .gpio_b = GPIO_ENCODER_0_B,
    .gpio_btn = GPIO_ENCODER_0_BTN,
    .btn_sticky = ENCODER_BTN_CLEAR,
    .dir_sticky = ENCODER_DIR_NONE,
    .var = NULL
};

encoder_t encoder_1 = {
    .gpio_a = GPIO_ENCODER_1_A,
    .gpio_b = GPIO_ENCODER_1_B,
    .gpio_btn = GPIO_ENCODER_1_BTN,
    .btn_sticky = ENCODER_BTN_CLEAR,
    .dir_sticky = ENCODER_DIR_NONE,
    .var = NULL
};

static encoder_t* get_encoder(uint32_t encoder) {
    switch (encoder) {
        case ENCODER_0:
            return &encoder_0;
        case ENCODER_1:
            return &encoder_1;
    }
    return NULL;
}

static void IRAM_ATTR encoder_btn_isr(void *encoder) {
    // TODO
    // Pin level is checked, since interrupt is firing on both push and release, probably due to debounce issues
    // Fix in HW, and remove check
    encoder_t *enc = get_encoder((uint32_t)encoder);

    if (gpio_get_level(enc->gpio_btn)) {
        enc->btn_sticky = ENCODER_BTN_SET;
    }
}

static void IRAM_ATTR encoder_dir_isr(void *encoder) {
    // TODO
    // Both pins are checked, since interrupt is firing multiple times per encoder tick, probably due to debounce issues
    // Fix in HW, and remove check on encoder A GPIO

    encoder_t *enc = get_encoder((uint32_t)encoder);
    uint32_t var_tmp;

    if (gpio_get_level(enc->gpio_a) == 0 && gpio_get_level(enc->gpio_b) == 1) {
        enc->dir_sticky = ENCODER_DIR_RIGHT;
        
        if (enc->var != NULL) {
            var_tmp = *enc->var + enc->step_size; 

            // Check for upper limit and overflow
            if (var_tmp > enc->high_limit || var_tmp < *enc->var) {
                (*enc->var) = enc->high_limit;
            } else {
                (*enc->var) = var_tmp;
            }
        }
    } else if (gpio_get_level(enc->gpio_a) == 0 && gpio_get_level(enc->gpio_b) == 0) {
        enc->dir_sticky = ENCODER_DIR_LEFT;

        if (enc->var != NULL) {
            var_tmp = *enc->var - enc->step_size;

            // Check for lower limit and underflow
            if (var_tmp < enc->low_limit || var_tmp > *enc->var) {
                (*enc->var) = enc->low_limit;
            } else {
                (*enc->var) = var_tmp;
            }
        }
    }
}

void encoder_setup() {
    gpio_config_t gpio_conf = {};
    gpio_conf.pin_bit_mask = (1ULL<<GPIO_ENCODER_0_A) | (1ULL<<GPIO_ENCODER_0_B) | (1ULL<<GPIO_ENCODER_0_BTN) | (1ULL<<GPIO_ENCODER_1_A) | (1ULL<<GPIO_ENCODER_1_B) | (1ULL<<GPIO_ENCODER_1_BTN);
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&gpio_conf);

    //TODO add physical pullups to board instead
//    gpio_set_pull_mode(GPIO_ENCODER_0_BTN, GPIO_PULLUP_ONLY);
//    gpio_set_pull_mode(GPIO_ENCODER_1_BTN, GPIO_PULLUP_ONLY);

    gpio_set_intr_type(GPIO_ENCODER_0_A, GPIO_INTR_ANYEDGE); //TODO GPIO_INTR_NEGEDGE
    gpio_set_intr_type(GPIO_ENCODER_1_A, GPIO_INTR_ANYEDGE); //TODO GPIO_INTR_NEGEDGE
    gpio_set_intr_type(GPIO_ENCODER_0_BTN, GPIO_INTR_POSEDGE);
    gpio_set_intr_type(GPIO_ENCODER_1_BTN, GPIO_INTR_POSEDGE);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(GPIO_ENCODER_0_A, encoder_dir_isr, (void*)ENCODER_0);
    gpio_isr_handler_add(GPIO_ENCODER_1_A, encoder_dir_isr, (void*)ENCODER_1);
    gpio_isr_handler_add(GPIO_ENCODER_0_BTN, encoder_btn_isr, (void*)ENCODER_0);
    gpio_isr_handler_add(GPIO_ENCODER_1_BTN, encoder_btn_isr, (void*)ENCODER_1);
}

uint32_t encoder_get_btn(uint32_t encoder) {
    encoder_t *enc = get_encoder(encoder);

    if (gpio_get_level(enc->gpio_btn)) {
        return ENCODER_BTN_SET;
    } else {
        return ENCODER_BTN_CLEAR;
    }
}

uint32_t encoder_get_btn_sticky(uint32_t encoder) {
    encoder_t *enc = get_encoder(encoder);

    uint32_t ret = enc->btn_sticky;
    enc->btn_sticky = ENCODER_DIR_NONE;
    return ret;
}

uint32_t encoder_get_dir_sticky(uint32_t encoder) {
    encoder_t *enc = get_encoder(encoder);

    uint32_t ret = enc->dir_sticky;
    enc->dir_sticky = ENCODER_DIR_NONE;
    return ret;
}

void encoder_register_var(uint32_t encoder, uint32_t *var, uint32_t step_size, uint32_t low_limit, uint32_t high_limit) {
    encoder_t *enc = get_encoder(encoder);

    enc->var = var;
    enc->step_size = step_size;
    enc->low_limit = low_limit;
    enc->high_limit = high_limit;
}

void encoder_unregister_var(uint32_t encoder) {
    encoder_t *enc = get_encoder(encoder);
    enc->var = NULL;
}
