#include "driver/gpio.h"

esp_err_t led_init(gpio_num_t led_pin);

esp_err_t led_switch(gpio_num_t led_pin, int level);