#include "oled.h"
#include <avr/io.h>

#define OLED_ADDRESS 0x3C

/* (7) Clear TWI Interrupt Flag, (6) Enable Acknowledge bit,
 * (5) START, (4) STOP, (3) Write Collision Flag is RO, (2) Enable TWI
 * Bit 1 is reserved, (0) disable TWI Interrupt
 */
#define TWCR_CONFIG 0b11000100

// Mask for reading the TWI Status Register
#define TWSR_TWS_MASK 0xF8

/* These are the lookup tables for writing characters to the screen.
 * Each character is 6x8 pixels. The bit order is from the bottom to
 * the top and left to right. The digits are represented directly by
 * their table index. Only the necessary letters are included so their
 * position is not related to their ASCII value.
 */
const uint8_t numbers_lookup[][6] = {
    {0b00111100, 0b01010010, 0b01001010, 0b01000110, 0b00111100, 0},
    {0b00000000, 0b01000100, 0b01111110, 0b01000000, 0b00000000, 0},
    {0b01000100, 0b01100010, 0b01010010, 0b01001010, 0b01000100, 0},
    {0b00100100, 0b01000010, 0b01001010, 0b01001010, 0b00110100, 0},
    {0b00110000, 0b00101000, 0b00100100, 0b01111110, 0b00100000, 0},
    {0b00101110, 0b01001010, 0b01001010, 0b01001010, 0b00110010, 0},
    {0b00111100, 0b01001010, 0b01001010, 0b01001010, 0b00110010, 0},
    {0b00000010, 0b00000010, 0b01110010, 0b00001010, 0b00000110, 0},
    {0b00110100, 0b01001010, 0b01001010, 0b01001010, 0b00110100, 0},
    {0b00100100, 0b01001010, 0b01001010, 0b01001010, 0b00111100, 0}
};

const uint8_t letters_lookup[][6] = {
    {0b01111100, 0b00001010, 0b00001010, 0b00001010, 0b01111100, 0}, // A - 0
    {0b01111110, 0b01001010, 0b01001010, 0b01001010, 0b00110100, 0}, // B - 1
    {0b01111110, 0b01001010, 0b01001010, 0b01001010, 0b01000010, 0}, // E - 2
    {0b01111110, 0b00001010, 0b00001010, 0b00001010, 0b00000010, 0}, // F - 3
    {0b01111110, 0b00001000, 0b00001000, 0b00001000, 0b01111110, 0}, // H - 4
    {0b00000000, 0b01000010, 0b01111110, 0b01000010, 0b00000000, 0}, // I - 5
    {0b01111110, 0b00000100, 0b00001000, 0b00000100, 0b01111110, 0}, // M - 6
    {0b01111110, 0b00001010, 0b00001010, 0b00001010, 0b01110100, 0}, // R - 7
    {0b01000100, 0b01001010, 0b01001010, 0b01001010, 0b00110010, 0}, // S - 8
    {0b01000100, 0b01100100, 0b01010100, 0b01001100, 0b01000100, 0}  // z - 9
};

const uint8_t symbols_lookup[][6] = {
    {0b00000000, 0b00000000, 0b00011000, 0b00011000, 0b00000000, 0}, // small dot   - 0
    {0b00000000, 0b00111100, 0b00111100, 0b00111100, 0b00111100, 0}, // large dot   - 1
    {0b00001000, 0b00011100, 0b00101010, 0b00001000, 0b00001000, 8}, // left arrow  - 2
    {0b00001000, 0b00001000, 0b00001000, 0b00101010, 0b00011100, 8}, // right arrow - 3
    {0b00001000, 0b00000100, 0b01111110, 0b00000100, 0b00001000, 0}, // up arrow    - 4
    {0b00010000, 0b00100000, 0b01111110, 0b00100000, 0b00010000, 0}  // down arrow  - 5
};


/* This function is used internaly to wait for the I2C interface
 * to stop transmitting, and check whether it was successful.
 * To prevent blocking for too long a timeout is added. A return
 * value of 0 means success, other values indicate an error.
 */
uint8_t _wait_TWCR(uint8_t cr_bit, uint8_t sr_value, uint8_t negate) {
    int timeout = 4000;
    while ((TWCR & (1 << cr_bit)) ? !negate : negate) {
        if (!(timeout--)) return 1; 
    }
    if (!((TWSR & TWSR_TWS_MASK) == sr_value)) return 2;
    return 0;
}

/* This function is used internally to send a single command to
 * the OLED controller. A return value of 0 means success, other
 * values indicate an error.
 */
uint8_t _send_command(uint8_t command) {
    TWDR = 0b10000000;     // Indicate next byte will be a command
    TWCR = TWCR_CONFIG; // Clears the interrupt flag and starts the transmission

    if(_wait_TWCR(TWINT, 0x28, 1)) return 1;

    TWDR = command;        // Load the command
    TWCR = TWCR_CONFIG; // Clears the interrupt flag and starts the transmission

    if(_wait_TWCR(TWINT, 0x28, 1)) return 2;

    return 0;
}

/* This function is used to begin an I2C transaction.
 * It sends a START condition, waits for an ACK, then it sends
 * the device address and waits for another ACK. A return value
 * of 0 means success, other values indicate an error.
 */
uint8_t i2c_start(void) {
    TWCR = TWCR_CONFIG | (1 << TWSTA);  // Send START
    
    if(_wait_TWCR(TWINT, 0x08, 1)) return 1;

    TWDR = (OLED_ADDRESS << 1);  // Send address
    TWCR = TWCR_CONFIG;

    if(_wait_TWCR(TWINT, 0x18, 1)) return 2;

    return 0;
}

/* This function is used to end an I2C transaction.
 * It sends a STOP condition. There is not ACK for the STOP
 * condition, so the function simply waits for the interface
 * to finish transmitting and then it returns.
 */
void i2c_stop(void) {
    TWCR = TWCR_CONFIG | (1 << TWSTO);  // Send STOP

    _wait_TWCR(TWSTO, 0, 0);    // We don't care about the return value
}

/* This function initializes the I2C (2-wire) interface and
 * configures the OLED display.
 */
uint8_t oled_init() {
    // I2C
    TWBR = 16;          // Bit rate register, target is ~100 kHz
    TWSR = 0;           // Set prescaler to 1
    
    if (i2c_start()) return 1;

    if(_send_command(0xD9)) return 2; // Set Pre-charge Period
    if(_send_command(0xF1)) return 2; // -Phase 2: 15 DCLK, Phase 1: 1 DCLK

    if(_send_command(0x8D)) return 2; // Set Charge Pump
    if(_send_command(0x14)) return 2; // -Enabled
    
    if(_send_command(0xDB)) return 2; // Set VCOMH Deselect Level
    if(_send_command(0x40)) return 2; // -0.89*Vcc

    if(_send_command(0xA1)) return 2; // Flip horizontally
    if(_send_command(0xC8)) return 2; // Flip vertically

    if(_send_command(0xAF)) return 2; // Display ON
    if(_send_command(0xA4)) return 2; // Use data from RAM

    if(_send_command(0x20)) return 2; // Set Memory Addressing Mode
    if(_send_command(0x00)) return 2; // -Horizontal

    i2c_stop();

    return 0;

}

/* This function is used to send a singe byte of display data
 * to the display. It can be used to send data for example in
 * a for loop, without calling i2c_start() and i2c_stop()
 * between each byte. Both "oled_raw" functions require the
 * START and STOP commands to be sent seperately.
 */
uint8_t oled_raw_write(uint8_t data) {
    TWDR = 0b11000000;
    TWCR = TWCR_CONFIG;

    if(_wait_TWCR(TWINT, 0x28, 1)) return 1;

    TWDR = data;
    TWCR = TWCR_CONFIG;
    
    if(_wait_TWCR(TWINT, 0x28, 1)) return 2;

    return 0;
}

/* This function sets the position at which new data will
 * be written. X can go up to 127 and y can go up to 7.
 * Both "oled_raw" functions require the START and STOP 
 * commands to be sent seperately.
 */
uint8_t oled_raw_set_position(uint8_t x, uint8_t y) {
    if(_send_command(0x21)) return 2; // Set column address
    if(_send_command(x)) return 2; // Start at x
    if(_send_command(0xff)) return 2; // End at 127
    if(_send_command(0x22)) return 2; // Set page address
    if(_send_command(y)) return 2; // Start at y
    if(_send_command(0x07)) return 2; // End at 7
    return 0;
}

/* This function writes a fixed length number to the display
 * at the specified coordinates. The number is converted to
 * decimal and padded with zeroes to fit the specified length.
 */
uint8_t oled_write_num_fixed(uint32_t n, uint8_t len, uint8_t x, uint8_t y, uint8_t invert) {
    if (i2c_start()) return 1;

    if (oled_raw_set_position(x, y)) return 2;

    TWDR = 0b01000000;
    TWCR = TWCR_CONFIG;

    if(_wait_TWCR(TWINT, 0x28, 1)) return 3;

    uint32_t dec = 1;
    while (--len > 0) {
        dec *= 10;
    }

    int i = 0;
    while (dec > 0) {
        int digit = n / dec;
        if (digit < 0) digit = 0;
        if (digit > 9) digit = 9;
        for (int j = 0; j < 6; j++) {
            TWDR = invert ? ~numbers_lookup[digit][j] : numbers_lookup[digit][j];
            TWCR = TWCR_CONFIG;
            
            if(_wait_TWCR(TWINT, 0x28, 1)) return 4;
        }

        n = n % dec;
        dec /= 10;
        i++;
    }

    i2c_stop();

    return 0;
}

/* This function writes a string of characters to the display
 * at the specified coordinates. The only valid characters in 
 * the input string are numeric and each digit represents 
 * a character in the lookup table.
 */
uint8_t oled_write_text(char *text, uint8_t x, uint8_t y, uint8_t invert) {
    if (i2c_start()) return 1;

    if (oled_raw_set_position(x, y)) return 2;

    TWDR = 0b01000000;
    TWCR = TWCR_CONFIG;

    if(_wait_TWCR(TWINT, 0x28, 1)) return 3;

    for (int i = 0; text[i] != 0; i++) {
        int letter = text[i] - '0';
        if (letter < 0) letter = 0;
        if (letter > 9) letter = 9;
        for (int j = 0; j < 6; j++) {
            TWDR = invert ? ~letters_lookup[letter][j] : letters_lookup[letter][j];
            TWCR = TWCR_CONFIG;
            
            if(_wait_TWCR(TWINT, 0x28, 1)) return 4;
        }
    }

    i2c_stop();

    return 0;
}

/* This function writes a string of symbols to the display
 * at the specified coordinates. The only valid characters in 
 * the input string are numeric 0-5 and each digit represents 
 * a symbol in the lookup table.
 */
uint8_t oled_write_symbol(char *symbols, uint8_t x, uint8_t y, uint8_t invert) {
    if (i2c_start()) return 1;

    if (oled_raw_set_position(x, y)) return 2;

    TWDR = 0b01000000;
    TWCR = TWCR_CONFIG;

    if(_wait_TWCR(TWINT, 0x28, 1)) return 3;

    for (int i = 0; symbols[i] != 0; i++) {
        int symbol = symbols[i] - '0';
        if (symbol < 0) symbol = 0;
        if (symbol > 9) symbol = 5;
        for (int j = 0; j < 6; j++) {
            TWDR = invert ? ~symbols_lookup[symbol][j] : symbols_lookup[symbol][j];
            TWCR = TWCR_CONFIG;
            
            if(_wait_TWCR(TWINT, 0x28, 1)) return 4;
        }
    }

    i2c_stop();

    return 0;
}
