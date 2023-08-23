#include <avr/eeprom.h>
#include "dalsik_eeprom.h"

static uint8_t read(int address) {
    return eeprom_read_byte((uint8_t*)(address));
}

static void write(int address, uint8_t value) {
    eeprom_write_byte((uint8_t*)address, value);
}

static void update(int address, uint8_t value) {
    uint8_t current_value = read(address);
    if (value != current_value) {
        write(address, value);
    }
}

// The keyboard side can be set during flashing. See dalsik.h
char EEPROM::get_keyboard_side() {
#ifdef SET_KEYBOARD_SIDE
    set_keyboard_side(SET_KEYBOARD_SIDE);
#endif

    return read(KEYBOARD_SIDE_ADDRESS);
}

void EEPROM::set_keyboard_side(char side) {
    if (side == KEYBOARD_SIDE_LEFT || side == KEYBOARD_SIDE_RIGHT) {
        update(KEYBOARD_SIDE_ADDRESS, side);
    }
}
