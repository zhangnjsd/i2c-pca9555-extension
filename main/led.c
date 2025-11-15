#include "led.h"

esp_err_t led_init(gpio_num_t led_pin)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << led_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK)
    {
        return ret;
    }
    return ESP_OK;
}

esp_err_t led_switch(gpio_num_t led_pin, int level)
{
    return gpio_set_level(led_pin, level);
}