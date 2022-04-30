#ifndef VIDEO_RX_H_INCLUDED
#define VIDEO_RX_H_INCLUDED

#include <stdint.h>

void video_rx_init_spi(void);
void video_rx_init_adc(void);
void video_rx_set_frequency(uint16_t freq);
uint8_t video_rx_get_rssi(void);

#endif