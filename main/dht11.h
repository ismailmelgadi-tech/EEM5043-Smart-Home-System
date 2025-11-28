#ifndef DHT11_H
#define DHT11_H

#include <stdbool.h>

void dht11_init_gpio(int gpio_num);
bool dht11_read_safe(float *temp, float *hum);

#endif