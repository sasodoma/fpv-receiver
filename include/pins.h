#ifndef PINS_H_INCLUDED
#define PINS_H_INCLUDED

#include <avr/io.h>

// Register level macro "functions"
#define _setGpioOutput(port, pin)         DDR##port |=  (1 << pin)
#define _setGpioInput(port, pin)          DDR##port &= ~(1 << pin), PORT##port &= ~(1 << pin)
#define _setGpioInputPullup(port, pin)    DDR##port &= ~(1 << pin), PORT##port |=  (1 << pin)
#define _setGpioHigh(port, pin)          PORT##port |=  (1 << pin)
#define _setGpioLow(port, pin)           PORT##port &= ~(1 << pin)

// Helper functions, so parameters match
#define setGpioOutput(...)        _setGpioOutput(__VA_ARGS__)
#define setGpioInput(...)         _setGpioInput(__VA_ARGS__)
#define setGpioInputPullup(...)   _setGpioInputPullup(__VA_ARGS__)
#define setGpioHigh(...)          _setGpioHigh(__VA_ARGS__)
#define setGpioLow(...)           _setGpioLow(__VA_ARGS__)

// Pin definitions
#define LED_BUILTIN D,1


#endif