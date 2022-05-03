#ifndef OLED_H_INCLUDED
#define OLED_H_INCLUDED

#include <stdint.h>

uint8_t i2c_start(void);
void i2c_stop(void);
uint8_t oled_init(void);
uint8_t oled_raw_write(uint8_t data);
uint8_t oled_raw_set_position(uint8_t x, uint8_t y);
uint8_t oled_write_num_fixed(uint32_t n, uint8_t len, uint8_t x, uint8_t y, uint8_t invert);
uint8_t oled_write_text(char *text, uint8_t x, uint8_t y, uint8_t invert);
uint8_t oled_write_symbol(char *symbols, uint8_t x, uint8_t y, uint8_t invert);

#endif