#include "i2c9555.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"

#define TAG "MAIN"

static bool button_sw = false;

static int device;

void i2c_io_callback(uint16_t pin, int level)
{
    ESP_LOGI(TAG, "I2C9555 IO Callback triggered");

    uint16_t pin_num = 0;
    for (int i = 0; i < 16; i++)
    {
        if (pin & (1 << i))
        {
            pin_num = i;
            break;
        }
    }

    ESP_LOGI(TAG, "Pin: %u, Level: %d", pin_num, level);

    switch (pin)
    {
    case EXT_IO1:
        level ? (button_sw = !button_sw) : 0;
        ESP_LOGI(TAG, "Button toggled, new state: %s", button_sw ? "ON" : "OFF");
        break;

    case EXT_IO10:
        led_switch(GPIO_NUM_4, level ? (button_sw ? true : false) : (button_sw ? false : true));
        ESP_LOGI(TAG, "LED switched to: %s", button_sw ? "ON" : "OFF");
        break;
    
    default:
        break;
    }
}

static void blink_test(void* args) {
    while (1) {
        i2c9555pin_write(device, EXT_IO16, HIGH);
        i2c9555pin_write(device, EXT_IO17, LOW);
        vTaskDelay(pdMS_TO_TICKS(1000));
        i2c9555pin_write(device, EXT_IO16, LOW);
        i2c9555pin_write(device, EXT_IO17, HIGH);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    led_init(GPIO_NUM_4);
    device = i2c9555_add_device(GPIO_NUM_9, GPIO_NUM_8, 0x20, GPIO_NUM_10, i2c_io_callback);
    vTaskDelay(pdMS_TO_TICKS(10));
    i2c9555_ioconfig(device, 0b0011111111111111);

    ESP_LOGI(TAG, "I2C9555 initialized");

    xTaskCreatePinnedToCore(blink_test, "blink_test", 4096, NULL, 2, NULL, 0);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
