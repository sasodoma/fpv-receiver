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

/* Macros for symbols, can be concatenated. For example:
 * OLED_A OLED_B OLED_C is equal to "012", which can be
 * used for oled_write_text().
 */ 
#define OLED_A  "0"
#define OLED_B  "1"
#define OLED_E  "2"
#define OLED_F  "3"
#define OLED_H  "4"
#define OLED_I  "5"
#define OLED_M  "6"
#define OLED_R  "7"
#define OLED_S  "8"
#define OLED_z  "9"

#define OLED_SMALL_DOT  "0"
#define OLED_LARGE_DOT  "1"
#define OLED_LEFT       "2"
#define OLED_RIGHT      "3"
#define OLED_UP         "4"
#define OLED_DOWN       "5"

#endif