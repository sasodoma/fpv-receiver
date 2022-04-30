#ifndef RTOS_H_INCLUDED
#define RTOS_H_INCLUDED

#include <stdint.h>

// Kazalec na funkcijo
typedef void (* ptr_function)(void);

typedef struct rtos_task {
    ptr_function driver;
    ptr_function init;
} rtos_task_t;

/* Sprejme velikost časovne rezine.
 * Konfigurira SysTick timer.
 * Vrne 0 če je ok, sicer je predolga rezina.
 */
uint8_t rtos_init(uint16_t slice_us);

/* Vklopi SysTick timer
 * (Požene RTOS)
 */
void rtos_enable(void);

/* Izklopi SysTick timer
 * (Ustavi RTOS)
 */
void rtos_disable(void);

#endif // RTOS_H_INCLUDED
