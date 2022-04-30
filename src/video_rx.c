#include "video_rx.h"
#include <avr/io.h>

/* This function is used internally to calculate the N
 * and A parameters for the RX chip, as specified in
 * the datasheet. The parameters are combined into a
 * value that can be sent over SPI. Due to the way the
 * frequency is set, the smallest change step is 2 Mhz.
 */
uint32_t _freq_to_data(uint16_t freq) {
	uint16_t f_lo_half = (freq - 479) >> 1;
	uint32_t N = f_lo_half / 32;
	uint8_t A = f_lo_half % 32;
	return (N << 7) | A;
}

/* This function initializes the output pins for
 * the SPI interface and sets up the SPI parameters.
 * The RX chip uses a 3-wire half duplex mode but
 * we only do writing so nothing special needs to
 * be done. The MISO pin is not used.
 */
void video_rx_init_spi() {
    // Set SS, MOSI and SCK as output.
	DDRB = (1 << DDB2) | (1 << DDB3) | (1 << DDB5);
	// Enable SPI, Master mode, clock /16, LSB first
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << DORD);
	// SS High
	PORTB = 1 << DDB2;
}

/* This function initializes the ADC and starts
 * a conversion, since the first conversion takes
 * longer than the later ones.
 */
void video_rx_init_adc() {
    // Enable ADC, set prescaler to /128 for 125 kHz
	ADCSRA = (1 << ADEN) | 0b111;
	// Set VCC as ADC reference, select channel 0
	ADMUX = (1 << REFS0);
	// Start first conversion
	ADCSRA |= (1 << ADSC);
	while (!(ADCSRA & (1 << ADIF)));
	// Clear ADIF flag
	ADCSRA |= (1 << ADIF);
}

/* This function writes a new frequency setting to
 * the RX. Note that every write causes a momentary
 * video loss so the frequency should only be written
 * when it actually changes.
 */
void video_rx_set_frequency(uint16_t freq) {
	// Combine the frequency data with the RW bit and the address.
    uint32_t data = _freq_to_data(freq);
	data = (data << 1) + 1; 	// Write
	data = (data << 4) + 0x1; 	// Synthesizer Register B

	PORTB = ~(1 << DDB2);
	for (int i = 0; i < 4; i++) {
		SPDR = data & 0xFF;
		data = data >> 8;
		/* Wait for transmission complete */
		while(!(SPSR & (1<<SPIF)));
	}
	PORTB = 1 << DDB2;
}

/* This function reads the ADC and converts the value to
 * an RSSI value. The RX datasheet does not specify an
 * exact conversion formula so the RSSI value is only
 * an indicator. Values range from 0 (bad) to 99 (good).
 */
uint8_t video_rx_get_rssi() {
    ADCSRA |= (1 << ADSC);
    while (!(ADCSRA & (1 << ADIF)));
    // Clear ADIF flag
    ADCSRA |= (1 << ADIF);

    // Read the results
    uint16_t adc_result = ADCL | (ADCH << 8);
    int16_t rssi = (adc_result - 130);
    if (rssi < 0) return 0;
    if (rssi > 99) return 99;
    return (uint8_t) rssi;
}