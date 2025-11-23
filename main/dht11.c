#include "dht11.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"
#include "esp_log.h"

static const char *TAG = "dht11";

static gpio_num_t dht_gpio;
static float last_temperature = 0.0;
static float last_humidity = 0.0;

// portmux for critical section
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void dht11_init(gpio_num_t gpio_num) {
    dht_gpio = gpio_num;
    // Configuration will be done in read function to switch between input/output
    // But we can set a pullup initially if we want, though the protocol handles it.
    gpio_reset_pin(dht_gpio);
    // Internal pull-up is good to have
    gpio_set_pull_mode(dht_gpio, GPIO_PULLUP_ONLY);
}

static int dht11_wait_for_level(int level, int timeout_us) {
    int count = 0;
    while (gpio_get_level(dht_gpio) == level) {
        if (count >= timeout_us) {
            return -1;
        }
        ets_delay_us(1);
        count++;
    }
    return count;
}

esp_err_t dht11_read(void) {
    uint8_t data[5] = {0, 0, 0, 0, 0};

    // Start signal
    gpio_set_direction(dht_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_gpio, 0);
    vTaskDelay(20 / portTICK_PERIOD_MS); // Low for at least 18ms
    gpio_set_level(dht_gpio, 1);
    ets_delay_us(30); // Pull up for 20-40us
    gpio_set_direction(dht_gpio, GPIO_MODE_INPUT);

    // Critical section start - timing sensitive
    portENTER_CRITICAL(&mux);

    // Wait for DHT response (Low 80us, High 80us)
    if (dht11_wait_for_level(1, 50) == -1) {
        portEXIT_CRITICAL(&mux);
        return ESP_FAIL; // Wait for previous high to end if any
    }
    if (dht11_wait_for_level(0, 100) == -1) {
        portEXIT_CRITICAL(&mux);
        return ESP_FAIL; // DHT pulls low
    }
    if (dht11_wait_for_level(1, 100) == -1) {
        portEXIT_CRITICAL(&mux);
        return ESP_FAIL; // DHT pulls high
    }

    // Read 40 bits
    for (int i = 0; i < 40; i++) {
        if (dht11_wait_for_level(0, 70) == -1) {
            portEXIT_CRITICAL(&mux);
            return ESP_FAIL; // Start of bit (50us low)
        }

        int duration = dht11_wait_for_level(1, 100); // Bit duration
        if (duration == -1) {
            portEXIT_CRITICAL(&mux);
            return ESP_FAIL;
        }

        if (duration > 40) { // 26-28us is 0, 70us is 1. Using 40 as threshold.
            data[i / 8] |= (1 << (7 - (i % 8)));
        }
    }

    portEXIT_CRITICAL(&mux);
    // Critical section end

    // Verify checksum
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        // Standard DHT11: Byte 0: Int RH, Byte 1: Dec RH, Byte 2: Int T, Byte 3: Dec T
        // Most DHT11 modules send 0 in decimal bytes.

        last_humidity = (float)data[0] + (float)data[1] / 10.0;
        last_temperature = (float)data[2] + (float)data[3] / 10.0;

        // Sanity check for DHT11 which often just sends int
        if (data[1] == 0 && data[3] == 0) {
             last_humidity = (float)data[0];
             last_temperature = (float)data[2];
        }

        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "Checksum failed");
        return ESP_FAIL;
    }
}

float dht11_get_temperature(void) {
    return last_temperature;
}

float dht11_get_humidity(void) {
    return last_humidity;
}
