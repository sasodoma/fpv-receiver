#ifndef BUTTONS_H_INCLUDED
#define BUTTONS_H_INCLUDED

#include <stdint.h>

void buttons_init(void);
uint8_t buttons_get_state(void);
uint8_t buttons_get_press(void);

#endif