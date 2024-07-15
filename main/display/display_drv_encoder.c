#include "driver/gpio.h"
#include "gpio_map.h"

#include "display_drv_encoder.h"

static int32_t encoder_diff;

static void dir_isr(void *encoder) {
    if (gpio_get_level(GPIO_ENCODER_0_A) == 0 && gpio_get_level(GPIO_ENCODER_0_B) == 1) {
 		encoder_diff++;
    } else if (gpio_get_level(GPIO_ENCODER_0_A) == 0 && gpio_get_level(GPIO_ENCODER_0_B) == 0) {
 		encoder_diff--;
    }
}

void display_drv_encoder_setup() {
    gpio_config_t gpio_conf = {};
    gpio_conf.pin_bit_mask = (1ULL<<GPIO_ENCODER_0_A) | (1ULL<<GPIO_ENCODER_0_B) | (1ULL<<GPIO_ENCODER_0_BTN);
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&gpio_conf);

    gpio_set_intr_type(GPIO_ENCODER_0_A, GPIO_INTR_ANYEDGE); //TODO GPIO_INTR_NEGEDGE

    gpio_install_isr_service(0);

    gpio_isr_handler_add(GPIO_ENCODER_0_A, dir_isr, NULL);
}

void display_drv_encoder_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    data->enc_diff = encoder_diff;
    encoder_diff = 0;

    if (gpio_get_level(GPIO_ENCODER_0_BTN)) {
    	data->state = LV_INDEV_STATE_PRESSED;
    } else {
    	data->state = LV_INDEV_STATE_RELEASED;
    }
}
