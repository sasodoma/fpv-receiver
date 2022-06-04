/* FM video receiver module with Arduino Nano.
 * Uses an RX5808 in SPI mode and an SSD1306 I2C OLED display
 * Connection:
 *  SSD1306:    SCK - A5
 *              SDA - A4
 *  RTC6715:    CH1 (Data) - D11
 *              CH2 (CS)   - D10
 *              CH3 (CLK)  - D13
 *              RSSI        - A0
 *  BUTTONS:    T1-T4 - D4-D7
 *              (buttons pull to GND)
 */

#include "pins.h"
#include "rtos.h"

int main(void) {
    /*************** INIT ***************/
    // GPIO
    setGpioOutput(LED_BUILTIN);
    setGpioLow(LED_BUILTIN);

    // RTOS
    rtos_init(5000);
    rtos_enable();

    for (;;) {
        // Not much to do here
    }

    while (1); // Safety net
}