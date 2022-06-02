#include <rtos_tasks.h>
#include <util/delay.h>
#include "oled.h"
#include "video_rx.h"
#include "buttons.h"

/* The bandplan with the most common bands:
 * A, B, E, F, R.
 * Each band has 8 channels.
 */
uint16_t bandplan[][8] = {
    {5865,5845,5825,5805,5785,5765,5745,5725},
    {5733,5752,5771,5790,5809,5828,5847,5866},
    {5705,5685,5665,5645,5885,5905,5925,5945},
    {5740,5760,5780,5800,5820,5840,5860,5880},
    {5658,5695,5732,5769,5806,5843,5880,5917}
};

/* Keeping track of our current position in the bandplan.
 * Setting rx_band to -1 means custom frequency is set.
 */
int rx_band = 4;
int rx_channel = 2;

/* Currently set frequency (in MHz) 
 * and RSSI (arbitrary units, 0-99)
 */
uint16_t freq = 5732;
uint8_t rssi = 0;


/* This task is responsible for updating the RX
 * frequency when it changes. It must update it
 * only on change because even updating with the
 * same value will cause a momentary loss of image.
 */
rtos_task_t task_rx_freq = {
    .init = init_rx_freq,
    .driver = driver_rx_freq
};

void init_rx_freq(void) {
    // RTC6715 - 3 wire SPI
    video_rx_init_spi();
    video_rx_set_frequency(freq);
}

void driver_rx_freq(void) {
    static uint16_t old_freq = 0;
    if (freq != old_freq) {
        video_rx_set_frequency(freq);
    }
    old_freq = freq;
}


/* This task is responsible for reading the ADC to
 * determine the current signal strength. The last
 * 16 readings are averaged to smooth out the value.
 */
rtos_task_t task_rx_rssi = {
    .init = init_rx_rssi,
    .driver = driver_rx_rssi
};

void init_rx_rssi(void) {
    video_rx_init_adc();
}

void driver_rx_rssi(void) {
    static int counter = 0;
    static uint8_t previous_rssi[16] = {0};
    previous_rssi[counter++ & 15] = video_rx_get_rssi();
    uint16_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += previous_rssi[i];
    }
    rssi = (uint8_t) (sum >> 4);
}


/* This task is responsible for updating the OLED
 * screen whenever the frequency or RSSI change.
 */
rtos_task_t task_oled = {
    .init = init_oled,
    .driver = driver_oled
};

void init_oled() {
    if(oled_init()) {
        while(1);
    }

    if (i2c_start()) {
        while(1);
    }

    for (int i = 0; i < 1024; i++) {
        oled_raw_write(0);
    }

    // Fill in some blank spaces so it looks better
    int fill_coords[] = {0,25,26,88,113,114,115,-1};

    for (int i = 0; fill_coords[i] != -1; i++) {
        oled_raw_set_position(fill_coords[i], 0);
        oled_raw_write(0xFF);
    }

    i2c_stop();

    // Write the top row text
    oled_write_num_fixed(freq, 4, 1, 0, 1);
    oled_write_num_fixed(rssi, 2, 128-6*2, 0, 1);
    oled_write_text(OLED_M OLED_H OLED_z, 27, 0, 1);
    oled_write_text(OLED_R OLED_S OLED_S OLED_I, 89, 0, 1);

    // Write the channel numbers
    for (int i = 1; i < 9; i++) {
        oled_write_num_fixed(i, 1, 12 + 12*i, 1, 0);
    }

    // Write the band letters
    char *channel_letters[] = {OLED_A,OLED_B,OLED_E,OLED_F,OLED_R,0};
    for (int i = 0; channel_letters[i] != 0; i++) {
        oled_write_text(channel_letters[i], 12, 2+i, 0);
    }

    // Write the square grid
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 5; y++) {
            if (x == 2 && y == 4) {
                oled_write_symbol(OLED_LARGE_DOT, 24 + 12*x, 2 + y, 0);
                continue;
            }
            oled_write_symbol(OLED_SMALL_DOT, 24 + 12*x, 2 + y, 0);
        }
    }

    // Write the arrows
    char *arrow_symbols[] = {OLED_LEFT,OLED_RIGHT,OLED_UP,OLED_DOWN,0};
    for (int i = 0; arrow_symbols[i] != 0; i++) {
        oled_write_symbol(arrow_symbols[i], 6 + 36*i, 7, 0);
    }
}

void driver_oled() {
    static uint8_t old_rx_band = 0;
    static uint8_t old_rx_channel = 0;
    static uint8_t old_rssi = 0;
    if (rx_band != old_rx_band || rx_channel != old_rx_channel) {
        oled_write_num_fixed(freq, 4, 1, 0, 1);
        oled_write_symbol(OLED_SMALL_DOT, 24 + 12*old_rx_channel, 2 + old_rx_band, 0);
        oled_write_symbol(OLED_LARGE_DOT, 24 + 12*rx_channel, 2 + rx_band, 0);
        old_rx_band = rx_band;
        old_rx_channel = rx_channel;
    }
    if (rssi != old_rssi) {
        oled_write_num_fixed(rssi, 2, 128-6*2, 0, 1);
        old_rssi = rssi;
    }
}


/* This task is responsible for reading the buttons
 * and updating the bandplan position and the frequency.
 */
rtos_task_t task_buttons = {
    .init = init_buttons,
    .driver = driver_buttons
};

void init_buttons() {
    buttons_init();
}

void driver_buttons() {
    uint8_t button_press = buttons_get_press();
    if (button_press & 1) rx_channel = rx_channel > 0 ? rx_channel - 1 : 7;
    if (button_press & 2) rx_channel = rx_channel < 7 ? rx_channel + 1 : 0;
    if (button_press & 4) rx_band = rx_band > 0 ? rx_band - 1 : 4;
    if (button_press & 8) rx_band = rx_band < 4 ? rx_band + 1 : 0;

    freq = bandplan[rx_band][rx_channel];
}


/* The list of tasks to be used by the RTOS.
 */
rtos_task_t *rtos_task_list[] = {
    &task_rx_freq, &task_rx_rssi, &task_oled, &task_buttons
, 0};
