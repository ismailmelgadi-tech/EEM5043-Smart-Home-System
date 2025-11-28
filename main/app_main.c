/* app_main.c - نسخة مستقرة بدون أخطاء Insights */
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
/* مكتبات RainMaker والشبكة */
#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h> 
#include <esp_rmaker_standard_params.h> 
#include <esp_rmaker_standard_devices.h> 
#include <esp_rmaker_utils.h>
#include <esp_rmaker_console.h>
#include <app_network.h> 
/* تم إزالة app_insights.h */

/* المكتبات المحلية */
#include "ssd1306.h"
#include "dht11.h"
#include "app_driver.h"

static const char *TAG = "SmartHome";

/* تعريفات الأرجل */
#define I2C_MASTER_SCL_IO           8
#define I2C_MASTER_SDA_IO           20
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000
#define DHT11_GPIO                  2
#define BTN_EMERGENCY_GPIO          10
#define SENSOR_READ_INTERVAL_MS     10000
#define AC_ON_TEMP                  27.0f
#define AC_OFF_TEMP                 26.0f
#define ALARM_ON_TEMP               30.0f
#define ALARM_OFF_TEMP              29.0f

/* متغيرات النظام */
static SemaphoreHandle_t xOLEDMutex;
static QueueHandle_t xSensorDataQueue;
static SSD1306_t oled_dev; 
#define MAX_QUEUE_SIZE 5

typedef struct { float temp; float hum; } sensor_data_t;

bool ac_state = false;
bool water_state = false;
bool sound_state = false;
bool led_state = false;
bool fan_state = false;
bool emergency_state = false; 

/* كائنات RainMaker */
esp_rmaker_device_t *ac_device = NULL;
esp_rmaker_device_t *water_device = NULL;
esp_rmaker_device_t *sound_device = NULL;
esp_rmaker_device_t *led_device = NULL;
esp_rmaker_device_t *fan_device = NULL;
esp_rmaker_device_t *emergency_device = NULL;
esp_rmaker_device_t *sensor_device = NULL;

/* --- دوال الهاردوير --- */
static void buttons_init(void) {
    gpio_config_t btn_conf = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BTN_EMERGENCY_GPIO), 
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE, 
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&btn_conf);
}

static void oled_init_custom(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    oled_dev.i2c_port = I2C_MASTER_NUM;
    oled_dev.address = 0x3C;
    oled_dev.width = 128;
    oled_dev.height = 32;
    ssd1306_init(&oled_dev, 128, 32);
    ssd1306_clear_screen(&oled_dev, false);
}

/* --- دوال RainMaker والمنطق --- */
void update_rmaker_state(const char *device_name, bool status) {
    const esp_rmaker_node_t *node = esp_rmaker_get_node();
    esp_rmaker_device_t *dev = esp_rmaker_node_get_device_by_name(node, device_name);
    if (dev && strcmp(esp_rmaker_device_get_name(dev), "Sensor") != 0) {
        esp_rmaker_param_t *param = esp_rmaker_device_get_param_by_name(dev, "Power");
        if (param) esp_rmaker_param_update_and_report(param, esp_rmaker_bool(status));
    }
}

void activate_emergency() {
    emergency_state = true;
    water_state = true; app_driver_set_water(true);
    sound_state = true; app_driver_set_sound(true);
    led_state = true;   app_driver_set_fire_led(true);
    fan_state = true;   app_driver_set_fan(true);
    ac_state = false;   app_driver_set_ac(false);
    update_rmaker_state("Fire Water", true);
    update_rmaker_state("Sound Alarm", true);
    update_rmaker_state("Fire LED", true);
    update_rmaker_state("Extractor Fan", true);
    update_rmaker_state("Emergency", true); 
    update_rmaker_state("Air Conditioner", false); 
    esp_rmaker_raise_alert("EMERGENCY ACTIVATED!");
}

void deactivate_emergency() {
    emergency_state = false;
    water_state = false; app_driver_set_water(false);
    sound_state = false; app_driver_set_sound(false);
    led_state = false;   app_driver_set_fire_led(false);
    fan_state = false;   app_driver_set_fan(false);
    update_rmaker_state("Fire Water", false);
    update_rmaker_state("Sound Alarm", false);
    update_rmaker_state("Fire LED", false);
    update_rmaker_state("Extractor Fan", false);
    update_rmaker_state("Emergency", false);
    esp_rmaker_raise_alert("Emergency Deactivated");
}

static esp_err_t write_cb(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
                          const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    const char *device_name = esp_rmaker_device_get_name(device);
    const char *param_name = esp_rmaker_param_get_name(param);
    if (strcmp(param_name, "Power") == 0) {
        if (strcmp(device_name, "Emergency") == 0) val.val.b ? activate_emergency() : deactivate_emergency();
        else if (strcmp(device_name, "Air Conditioner") == 0) { ac_state = val.val.b; app_driver_set_ac(ac_state); }
        else if (strcmp(device_name, "Fire Water") == 0) { water_state = val.val.b; app_driver_set_water(water_state); }
        else if (strcmp(device_name, "Sound Alarm") == 0) { sound_state = val.val.b; app_driver_set_sound(sound_state); }
        else if (strcmp(device_name, "Fire LED") == 0) { led_state = val.val.b; app_driver_set_fire_led(led_state); }
        else if (strcmp(device_name, "Extractor Fan") == 0) { fan_state = val.val.b; app_driver_set_fan(fan_state); }
        
        if (strcmp(device_name, "Emergency") != 0) esp_rmaker_param_update_and_report(param, val);
    }
    return ESP_OK;
}

static void task_sensor_reader(void *pvParameters) {
    sensor_data_t data;
    float temp_val = 0.0f, hum_val = 0.0f;
    while (1) {
        if (dht11_read_safe(&temp_val, &hum_val)) {
            data.temp = temp_val; data.hum = hum_val;
            xQueueSend(xSensorDataQueue, &data, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_INTERVAL_MS));
    }
}

static void task_system_controller(void *pvParameters) {
    sensor_data_t current_data = {0.0f, 0.0f};
    char line1[32], line2[32];
    bool auto_ac_active = false, auto_alarm_active = false;
    while (1) {
        if (xQueueReceive(xSensorDataQueue, &current_data, portMAX_DELAY) == pdPASS) {
            float g_temp = current_data.temp;
            float g_hum = current_data.hum;
            
            if (g_temp >= AC_ON_TEMP && !ac_state && !emergency_state) { 
                ac_state = true; app_driver_set_ac(true); update_rmaker_state("Air Conditioner", true); auto_ac_active = true;
            } else if (g_temp <= AC_OFF_TEMP && auto_ac_active && !emergency_state) {
                ac_state = false; app_driver_set_ac(false); update_rmaker_state("Air Conditioner", false); auto_ac_active = false;
            }
            if (g_temp >= ALARM_ON_TEMP && !emergency_state) {
                activate_emergency(); auto_alarm_active = true;
            } else if (g_temp <= ALARM_OFF_TEMP && auto_alarm_active && emergency_state) { 
                deactivate_emergency(); auto_alarm_active = false;
            }

            if (xSemaphoreTake(xOLEDMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                ssd1306_clear_screen(&oled_dev, false);
                snprintf(line1, sizeof(line1), "T:%.1fC H:%.0f%%", g_temp, g_hum);
                ssd1306_display_text(&oled_dev, 0, line1, strlen(line1), false);
                if (emergency_state) ssd1306_display_text(&oled_dev, 2, "!! EMERGENCY !!", 15, false);
                else {
                    snprintf(line2, sizeof(line2), "A%d W%d S%d F%d L%d", ac_state, water_state, sound_state, fan_state, led_state);
                    ssd1306_display_text(&oled_dev, 2, line2, strlen(line2), false);
                }
                xSemaphoreGive(xOLEDMutex);
            }
            if (sensor_device) {
                esp_rmaker_param_update_and_report(esp_rmaker_device_get_param_by_name(sensor_device, "Temperature"), esp_rmaker_float(g_temp));
                esp_rmaker_param_update_and_report(esp_rmaker_device_get_param_by_name(sensor_device, "Humidity"), esp_rmaker_float(g_hum));
            }
        }
    }
}

static void task_emergency_monitor(void *pvParameters) {
    bool btn_state_prev = true; 
    while (1) {
        bool btn_state_curr = gpio_get_level(BTN_EMERGENCY_GPIO);
        if (btn_state_prev == true && btn_state_curr == false) (!emergency_state) ? activate_emergency() : deactivate_emergency();
        btn_state_prev = btn_state_curr;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_main() {
    /* 1. تهيئة الهاردوير */
    app_driver_init();
    buttons_init();
    oled_init_custom();
    dht11_init_gpio(DHT11_GPIO);

    /* 2. تهيئة الذاكرة NVS */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /* 3. تهيئة الشبكة (الطريقة الصحيحة) */
    app_network_init();

    /* 4. تهيئة RainMaker */
    esp_rmaker_config_t rainmaker_cfg = {
        .enable_time_sync = false,
    };
    esp_rmaker_node_t *node = esp_rmaker_node_init(&rainmaker_cfg, "Smart Home System", "ESP32-C3 Controller");
    if (!node) {
        ESP_LOGE(TAG, "Could not initialise node. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }

    /* 5. إنشاء الأجهزة */
    ac_device = esp_rmaker_device_create("Air Conditioner", "esp.device.fan", NULL);
    esp_rmaker_device_add_cb(ac_device, write_cb, NULL);
    esp_rmaker_device_add_param(ac_device, esp_rmaker_power_param_create("Power", false));
    esp_rmaker_node_add_device(node, ac_device);

    water_device = esp_rmaker_device_create("Fire Water", "esp.device.switch", NULL);
    esp_rmaker_device_add_cb(water_device, write_cb, NULL);
    esp_rmaker_device_add_param(water_device, esp_rmaker_power_param_create("Power", false));
    esp_rmaker_node_add_device(node, water_device);

    sound_device = esp_rmaker_device_create("Sound Alarm", "esp.device.switch", NULL);
    esp_rmaker_device_add_cb(sound_device, write_cb, NULL);
    esp_rmaker_device_add_param(sound_device, esp_rmaker_power_param_create("Power", false));
    esp_rmaker_node_add_device(node, sound_device);

    led_device = esp_rmaker_device_create("Fire LED", "esp.device.lightbulb", NULL);
    esp_rmaker_device_add_cb(led_device, write_cb, NULL);
    esp_rmaker_device_add_param(led_device, esp_rmaker_power_param_create("Power", false));
    esp_rmaker_node_add_device(node, led_device);

    fan_device = esp_rmaker_device_create("Extractor Fan", "esp.device.fan", NULL);
    esp_rmaker_device_add_cb(fan_device, write_cb, NULL);
    esp_rmaker_device_add_param(fan_device, esp_rmaker_power_param_create("Power", false));
    esp_rmaker_node_add_device(node, fan_device);

    emergency_device = esp_rmaker_device_create("Emergency", "esp.device.switch", NULL);
    esp_rmaker_device_add_cb(emergency_device, write_cb, NULL);
    esp_rmaker_device_add_param(emergency_device, esp_rmaker_power_param_create("Power", false));
    esp_rmaker_node_add_device(node, emergency_device);

    sensor_device = esp_rmaker_device_create("Sensor", "esp.device.sensor", NULL);
    esp_rmaker_device_add_param(sensor_device, esp_rmaker_param_create("Temperature", "esp.param.temperature", esp_rmaker_float(0), PROP_FLAG_READ));
    esp_rmaker_device_add_param(sensor_device, esp_rmaker_param_create("Humidity", "esp.param.humidity", esp_rmaker_float(0), PROP_FLAG_READ));
    esp_rmaker_node_add_device(node, sensor_device);

    /* 6. تشغيل الخدمات (بدون Insights) */
    esp_rmaker_ota_enable_default();
    esp_rmaker_start();

    /* 7. بدء المهام */
    xOLEDMutex = xSemaphoreCreateMutex();
    xSensorDataQueue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(sensor_data_t));

    xTaskCreate(task_sensor_reader, "SensorTask", 3072, NULL, 5, NULL);
    xTaskCreate(task_system_controller, "ControllerTask", 4096, NULL, 4, NULL);
    xTaskCreate(task_emergency_monitor, "EmergencyMonitor", 2048, NULL, 3, NULL);

    /* 8. بدء التزويد (QR Code) */
    err = app_network_start(POP_TYPE_MAC);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not start Wifi. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }
}