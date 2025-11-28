#ifndef SSD1306_H
#define SSD1306_H

#include "driver/i2c.h"

typedef struct {
    i2c_port_t i2c_port;
    uint8_t address;
    uint8_t width;
    uint8_t height;
} SSD1306_t;

void ssd1306_init(SSD1306_t *dev, uint8_t width, uint8_t height);
void ssd1306_clear_screen(SSD1306_t *dev, bool invert);
void ssd1306_display_text(SSD1306_t *dev, int page, char *text, int text_len, bool invert);

#endif