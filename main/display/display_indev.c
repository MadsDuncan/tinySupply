#include "driver/gpio.h"
#include "gpio_map.h"

#include "display_internal.h"

static int32_t enc_diff = 0;
static uint8_t btn_state = LV_INDEV_STATE_RELEASED;

static void btn_isr() {
    if (gpio_get_level(GPIO_ENCODER_0_BTN) == 1) {
        btn_state = LV_INDEV_STATE_PRESSED;
    } else {
        btn_state = LV_INDEV_STATE_RELEASED;
    }
}

static void dir_isr() {
    if (gpio_get_level(GPIO_ENCODER_0_A) == 0 && gpio_get_level(GPIO_ENCODER_0_B) == 1) {
 		enc_diff++;
    } else if (gpio_get_level(GPIO_ENCODER_0_A) == 0 && gpio_get_level(GPIO_ENCODER_0_B) == 0) {
 		enc_diff--;
    }
}

void display_indev_setup() {
    gpio_config_t gpio_conf = {};
    gpio_conf.pin_bit_mask = (1ULL<<GPIO_ENCODER_0_A) | (1ULL<<GPIO_ENCODER_0_B) | (1ULL<<GPIO_ENCODER_0_BTN);
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&gpio_conf);

    gpio_install_isr_service(0);
    gpio_set_intr_type(GPIO_ENCODER_0_A, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(GPIO_ENCODER_0_A, dir_isr, NULL);
    gpio_set_intr_type(GPIO_ENCODER_0_BTN, GPIO_INTR_ANYEDGE);
    gpio_isr_handler_add(GPIO_ENCODER_0_BTN, btn_isr, NULL);
}

void display_indev_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    data->state = btn_state;
    data->enc_diff = enc_diff;
    enc_diff = 0;
}

void display_indev_touch_btn_cb(lv_event_t *event) {
    switch ((uint32_t)event->user_data) {
        case DISPLAY_INDEV_TOUCH_BTN_MENU:

            break;
        case DISPLAY_INDEV_TOUCH_BTN_UP:
            if (event->code == LV_EVENT_PRESSED || event->code == LV_EVENT_LONG_PRESSED_REPEAT) {
                enc_diff++;
            }
            break;
        case DISPLAY_INDEV_TOUCH_BTN_DOWN:
            if (event->code == LV_EVENT_PRESSED || event->code == LV_EVENT_LONG_PRESSED_REPEAT) {
                enc_diff--;
            }
            break;
        case DISPLAY_INDEV_TOUCH_BTN_ENTER:
            if (event->code ==  LV_EVENT_PRESSED) {
                btn_state = LV_INDEV_STATE_PRESSED;
            } else if (event->code ==  LV_EVENT_RELEASED) {
                btn_state = LV_INDEV_STATE_RELEASED;
            }
            break;
    }
}