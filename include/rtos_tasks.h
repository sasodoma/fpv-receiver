#ifndef RTOS_TASKS_H_INCLUDED
#define RTOS_TASKS_H_INCLUDED

#include <rtos.h>
void init_rx_freq();
void driver_rx_freq();
void init_rx_rssi();
void driver_rx_rssi();
void init_oled();
void driver_oled();
void init_buttons();
void driver_buttons();

extern rtos_task_t *rtos_task_list[];

#endif
