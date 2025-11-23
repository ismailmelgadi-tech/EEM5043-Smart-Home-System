/* Temperature Sensor demo implementation using RGB LED and timer

    This example code is in the Public Domain (or CC0 licensed, at your option.)

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <sdkconfig.h>
#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h> 
#include <esp_rmaker_standard_params.h> 
#include <esp_log.h>

#include <app_reset.h>
#include <ws2812_led.h>
#include "app_priv.h"
#include "dht11.h"

static const char *TAG = "app_driver";

/* This is the button that is used for toggling the power */
#define BUTTON_GPIO          CONFIG_EXAMPLE_BOARD_BUTTON_GPIO
#define BUTTON_ACTIVE_LEVEL  0

/* GPIO for DHT11 */
#define DHT_GPIO       2

#define DEFAULT_SATURATION  100
#define DEFAULT_BRIGHTNESS  50

#define WIFI_RESET_BUTTON_TIMEOUT       3
#define FACTORY_RESET_BUTTON_TIMEOUT    10

static uint16_t g_hue;
static uint16_t g_saturation = DEFAULT_SATURATION;
static uint16_t g_value = DEFAULT_BRIGHTNESS;

// Global state
static float g_temperature = DEFAULT_TEMPERATURE;
static float g_humidity = 0.0;
static bool g_switch_state = true; // Actuator state

// Queue for sensor data
static QueueHandle_t sensor_queue;

// Task Handles
static TaskHandle_t task_handle_sensor = NULL;
static TaskHandle_t task_handle_cloud = NULL;
static TaskHandle_t task_handle_alert = NULL;

// --- Actuator Callback (Enables Task 4: Voice Control - "Turn on/off") ---
static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }

    if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_POWER_NAME) == 0) {
        // --- تصحيح: استخدام val.val.b ---
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                val.val.b ? "true" : "false", esp_rmaker_device_get_name(device),
                esp_rmaker_param_get_name(param));

        // --- تصحيح: استخدام val.val.b ---
        g_switch_state = val.val.b;

        if (g_switch_state) {
            // Restore color
            ws2812_led_set_hsv(g_hue, g_saturation, g_value);
        } else {
            // Turn off LED
            ws2812_led_clear();
        }

        esp_rmaker_param_update_and_report(param, val);
    }
    return ESP_OK;
}

// --- Task 1: Sensor Reading Task ---
// Runs periodically to collect sensor data
static void app_sensor_reading_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Task 1 (Sensor Reader) started");
    while(1) {
        sensor_data_t data;
        if (dht11_read() == ESP_OK) {
            data.temperature = dht11_get_temperature();
            data.humidity = dht11_get_humidity();

            // Update globals for getters
            g_temperature = data.temperature;
            g_humidity = data.humidity;

            // Send to Queue
            // We use overwrite if queue is full to ensure latest data is processed
            xQueueOverwrite(sensor_queue, &data);
        } else {
            ESP_LOGW(TAG, "DHT11 Read Failed");
        }

        // -----------------------------------------------------------
        // تم التعديل هنا: الانتظار لمدة 5000 ميلي ثانية (5 ثواني)
        // -----------------------------------------------------------
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

// --- Task 2: Cloud Communication Task ---
// Handles sending data to RainMaker
static void app_cloud_comm_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Task 2 (Cloud Comm) started");
    sensor_data_t data;
    while(1) {
        // Wait for data from queue
        if (xQueueReceive(sensor_queue, &data, portMAX_DELAY) == pdTRUE) {

            // 1. Update Cloud
            if (temp_sensor_device) {
                esp_rmaker_param_update_and_report(
                    esp_rmaker_device_get_param_by_type(temp_sensor_device, ESP_RMAKER_PARAM_TEMPERATURE),
                    esp_rmaker_float(data.temperature));

                esp_rmaker_param_t *hum_param = esp_rmaker_device_get_param_by_name(temp_sensor_device, "Humidity");
                if (hum_param) {
                    esp_rmaker_param_update_and_report(hum_param, esp_rmaker_float(data.humidity));
                }
            }

            // 2. Update Local Actuator (LED) visualization
            float temp_for_hue = data.temperature;
            if (temp_for_hue < 0) temp_for_hue = 0;
            if (temp_for_hue > 100) temp_for_hue = 100;
            g_hue = (100 - (int)temp_for_hue) * 2;

            if (g_switch_state) {
                 ws2812_led_set_hsv(g_hue, g_saturation, g_value);
            }

            // Notify Alert Task
            xTaskNotify(task_handle_alert, 0, eNoAction);
        }
    }
}

// --- Task 3: Push Notification Task ---
// Checks if a notification should be sent
static void app_push_notification_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Task 3 (Push Notification) started");
    while(1) {
        // Wait for notification from Cloud Task
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Check threshold
        if (g_temperature > 40.0) {
            ESP_LOGW(TAG, "CRITICAL: Temperature %.1f > 40.0! Sending Alert.", g_temperature);
            esp_rmaker_raise_alert("Warning! High Temperature Detected");
        }
    }
}

float app_get_current_temperature()
{
    return g_temperature;
}

float app_get_current_humidity()
{
    return g_humidity;
}

bool app_get_switch_state()
{
    return g_switch_state;
}

esp_err_t app_sensor_init(void)
{
    esp_err_t err = ws2812_led_init();
    if (err != ESP_OK) {
        return err;
    }

    dht11_init(DHT_GPIO);

    // Create Queue (Size 1 is enough for latest value)
    sensor_queue = xQueueCreate(1, sizeof(sensor_data_t));

    // Create Tasks
    xTaskCreate(app_sensor_reading_task, "sensor_task", 4096, NULL, 5, &task_handle_sensor);
    xTaskCreate(app_cloud_comm_task, "cloud_task", 4096, NULL, 4, &task_handle_cloud);
    xTaskCreate(app_push_notification_task, "alert_task", 4096, NULL, 3, &task_handle_alert);

    return ESP_OK;
}

void app_driver_init()
{
    // Initialize sensors, LEDs, and tasks
    app_sensor_init();

    // Initialize reset button
    app_reset_button_register(app_reset_button_create(BUTTON_GPIO, BUTTON_ACTIVE_LEVEL),
                WIFI_RESET_BUTTON_TIMEOUT, FACTORY_RESET_BUTTON_TIMEOUT);
}

// Expose write callback for app_main to register
esp_err_t app_switch_write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx) {
    return write_cb(device, param, val, priv_data, ctx);
}