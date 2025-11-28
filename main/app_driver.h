#pragma once
#include <stdbool.h>

// تعريف الدوال الموجودة في app_driver.c
void app_driver_init(void);
void app_driver_set_ac(bool state);
void app_driver_set_water(bool state);
void app_driver_set_sound(bool state);
void app_driver_set_fire_led(bool state);
void app_driver_set_fan(bool state);
void app_driver_set_alarm(bool state); // دالة التوافق
float app_driver_get_temp(void);
float app_driver_get_humidity(void);