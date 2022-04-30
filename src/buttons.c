#include "buttons.h"
#include <avr/io.h>

/* This function initializes pins D4-D7 as
 * pulled-up inputs.
 */
void buttons_init(void) {
    // Pins 4-7 as inputs
    DDRD &= 0x0f;
    // Enable pullups
    PORTD |= 0xf0;
}

/* This function returns the current
 * state of the buttons.
 */
uint8_t buttons_get_state(void) {
    return ((~PIND) >> 4) & 0x0F;
}

/* This function returns a value indicating
 * whether a button has been pressed since it
 * was last called. (rising edge detection)
 */
uint8_t buttons_get_press(void) {
    static uint8_t lt;

    uint8_t t = buttons_get_state();
    uint8_t state = t & ~lt;

    lt = t;
    return state;
}
