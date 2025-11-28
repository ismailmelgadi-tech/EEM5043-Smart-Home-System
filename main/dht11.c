#include "dht11.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"

static gpio_num_t dht_gpio = GPIO_NUM_2;

void dht11_init_gpio(int gpio_num) {
    dht_gpio = (gpio_num_t)gpio_num;
    gpio_reset_pin(dht_gpio);
    gpio_set_pull_mode(dht_gpio, GPIO_PULLUP_ONLY);
}

static int wait_for_level(int level, int timeout_us) {
    int count = 0;
    while (gpio_get_level(dht_gpio) == level) {
        if (count >= timeout_us) return -1;
        esp_rom_delay_us(1);
        count++;
    }
    return count;
}

bool dht11_read_safe(float *temp, float *hum) {
    uint8_t data[5] = {0,0,0,0,0};
    
    gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_gpio, 0);
    esp_rom_delay_us(20000);
    gpio_set_level(dht_gpio, 1);
    esp_rom_delay_us(40);
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT);

    if (wait_for_level(0, 85) == -1) return false;
    if (wait_for_level(1, 85) == -1) return false;

    for (int i = 0; i < 40; i++) {
        if (wait_for_level(0, 75) == -1) return false;
        int duration = wait_for_level(1, 100);
        if (duration == -1) return false;
        if (duration > 40) data[i/8] |= (1 << (7-(i%8)));
    }

    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        *hum = (float)data[0] + (float)data[1] * 0.1;
        *temp = (float)data[2] + (float)data[3] * 0.1;
        return true;
    }
    return false;
}