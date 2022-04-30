#include <rtos.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <rtos_tasks.h>
#include "pins.h"

/* This function calculates the slice duration in
 * timer ticks, call the init function of every task
 * and sets up the timer and its interrupt.
 */
uint8_t rtos_init(uint16_t slice_us){

    uint32_t slice_ticks = slice_us << 1;

    if (slice_ticks > 0x10000) return 1;

    // Initialize all tasks
    for (int i = 0; rtos_task_list[i] != 0; i++) {
        rtos_task_list[i]->init();
    }

    // Initialize Timer 1
    // CTC mode, TOP = ICR1
    TCCR1A = 0;
    TCCR1B = (1 << WGM13) | (1 << WGM12);
    // Write TOP value into ICR1
    ICR1 = slice_ticks - 1;
    // Enable interrupt
    TIMSK1 = (1 << ICIE1);
    TCNT1 = 0;
    sei();

    return 0;
}

/* This function enables the timer by changing its clock
 * source from none to clk/8 prescaler.
 */
void rtos_enable(void) {
    TCCR1B |= (1 << CS11);
}

/* This function disables the timer by changing its clock
 * source from clk/8 prescaler to none.
 */
void rtos_disable(void) {
    TCCR1B &= ~(1 << CS11);
}

/* This is the interrupt handler routine which is called at
 * the beginning of every time slice. The scheduler cycles
 * through the task list and calls the driver functions.
 * If a task takes too long to execute the handler enters
 * an infinite loop and blinks the status LED.
 */
ISR(TIMER1_CAPT_vect) {
    static uint8_t i = 0;
    rtos_task_list[i]->driver();
    if (rtos_task_list[++i] == 0) {i = 0;}

    if (TIFR1 & (1 << ICF1)) {
        // Missed an interrupt, commit seppuku.
        while(1) {
            setGpioHigh(LED_BUILTIN);
            _delay_ms(100);
            setGpioLow(LED_BUILTIN);
            _delay_ms(100);
        }
    }
}

