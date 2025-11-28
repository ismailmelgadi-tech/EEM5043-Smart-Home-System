/* app_driver.c - Hardware Driver */
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdbool.h>

#define AC_GPIO         3
#define WATER_GPIO      4
#define SOUND_GPIO      5
#define FIRE_LED_GPIO   6
#define FAN_GPIO        7

static const char *TAG = "app_driver";

void app_driver_init()
{
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << AC_GPIO) | (1ULL << WATER_GPIO) | 
                        (1ULL << SOUND_GPIO) | (1ULL << FIRE_LED_GPIO) | 
                        (1ULL << FAN_GPIO),
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
    
    // استخدام TAG لإزالة التحذير
    ESP_LOGI(TAG, "Hardware driver initialized successfully on GPIO 3-7.");

    gpio_set_level(AC_GPIO, 0);
    gpio_set_level(WATER_GPIO, 0);
    gpio_set_level(SOUND_GPIO, 0);
    gpio_set_level(FIRE_LED_GPIO, 0);
    gpio_set_level(FAN_GPIO, 0);
}

void app_driver_set_ac(bool state) { gpio_set_level(AC_GPIO, state); }
void app_driver_set_water(bool state) { gpio_set_level(WATER_GPIO, state); }
void app_driver_set_sound(bool state) { gpio_set_level(SOUND_GPIO, state); }
void app_driver_set_fire_led(bool state) { gpio_set_level(FIRE_LED_GPIO, state); }
void app_driver_set_fan(bool state) { gpio_set_level(FAN_GPIO, state); }

// Compatibility Wrapper
void app_driver_set_alarm(bool state) {
    app_driver_set_sound(state);
    app_driver_set_fire_led(state);
}

float app_driver_get_temp() { return 0.0; }
float app_driver_get_humidity() { return 0.0; }