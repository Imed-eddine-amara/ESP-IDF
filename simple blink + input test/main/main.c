#include "driver/gpio.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define BLINK_GPIO 14 // Use standard pin definitions
#define BUTTON_GPIO 12 
    // 1. Configure the GPIO To Output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BLINK_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    // 1. Configure the GPIO To INPUT
    gpio_config_t io_conf2 = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
void app_main(void) {

    gpio_config(&io_conf);
    gpio_config(&io_conf2);
    // 2. Use the pin
    while (1) {
        gpio_set_level(BLINK_GPIO, 1); // Turn ON
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_set_level(BLINK_GPIO, 0); // Turn OFF
        vTaskDelay(pdMS_TO_TICKS(1000));
        printf("Pin %d state is: %d\n", BUTTON_GPIO,gpio_get_level(BUTTON_GPIO)); //test button on gpio 12
    }
}
