#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <esp_rmaker_core.h>

#define DEFAULT_TEMPERATURE 25.0
#define REPORTING_PERIOD    60 /* Seconds */

// Data structure for inter-task communication
typedef struct {
    float temperature;
    float humidity;
} sensor_data_t;

extern esp_rmaker_device_t *temp_sensor_device;
extern esp_rmaker_device_t *switch_device;

void app_driver_init(void);
float app_get_current_temperature();
float app_get_current_humidity();
bool app_get_switch_state();
esp_err_t app_switch_write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx);
