#ifndef DHT11_H
#define DHT11_H

#include "esp_err.h"
#include "driver/gpio.h"

/**
 * @brief Initialize DHT11 sensor
 *
 * @param gpio_num GPIO number where the sensor is connected
 */
void dht11_init(gpio_num_t gpio_num);

/**
 * @brief Read data from DHT11 sensor
 *
 * @return esp_err_t ESP_OK on success, ESP_FAIL on checksum error or timeout
 */
esp_err_t dht11_read(void);

/**
 * @brief Get the last read temperature
 *
 * @return float Temperature in Celsius
 */
float dht11_get_temperature(void);

/**
 * @brief Get the last read humidity
 *
 * @return float Humidity in percentage
 */
float dht11_get_humidity(void);

#endif // DHT11_H
