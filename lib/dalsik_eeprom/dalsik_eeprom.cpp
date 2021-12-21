#include <avr/eeprom.h>
#include "dalsik.h"
#include "dalsik_eeprom.h"

static uint16_t length() {
    return E2END + 1;
}

#define KEYBOARD_SIDE_EEPROM_ADDRESS length() - 1

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

static int get_eeprom_address(uint8_t layer, KeyCoords c) {
    return sizeof(EEPROM_KeyInfo)*(
        layer*KEY_COUNT + c.row*2*ONE_SIDE_COL_PIN_COUNT + c.col
    );
}

static int get_tapdance_eeprom_address(uint8_t index, uint8_t tap) {
    return TAPDANCE_EEPROM_OFFSET
        + sizeof(EEPROM_KeyInfo)*(index*MAX_TAPDANCE_TAPS + tap-1);
}

void EEPROM::clear_all() {
    for (uint32_t i = 0; i < length(); i++) {
        update(i, 0x00);
    }
}

void EEPROM::clear_keymap() {
    for (uint32_t i = 0; i < TAPDANCE_EEPROM_OFFSET; i++) {
        update(i, 0x00);
    }
}

void EEPROM::clear_tapdance() {
    for (uint32_t i = TAPDANCE_EEPROM_OFFSET; i < length(); i++) {
        update(i, 0x00);
    }
}

uint8_t EEPROM::get_keyboard_side() {
    return read(KEYBOARD_SIDE_EEPROM_ADDRESS);
}

void EEPROM::set_keyboard_side(uint8_t side) {
    update(KEYBOARD_SIDE_EEPROM_ADDRESS, side);
}

EEPROM_KeyInfo EEPROM::get_key(uint8_t layer, KeyCoords c) {
    int address  = get_eeprom_address(layer, c);
    uint8_t type = read(address + 0x00);
    uint8_t key  = read(address + 0x01);

    return EEPROM_KeyInfo { type, key };
}

EEPROM_KeyInfo EEPROM::get_tapdance_key(uint8_t index, uint8_t tap) {
    int address  = get_tapdance_eeprom_address(index, tap);
    uint8_t type = read(address + 0x00);
    uint8_t key  = read(address + 0x01);

    return EEPROM_KeyInfo { type, key };
}

void EEPROM::set_key(uint8_t layer, KeyInfo key_info) {
    int address = get_eeprom_address(layer, key_info.coords);

    update(address + 0x00, key_info.type);
    update(address + 0x01, key_info.key);
}

void EEPROM::set_tapdance_key(uint8_t index, uint8_t tap, KeyInfo key_info) {
    int address = get_tapdance_eeprom_address(index, tap);

    update(address + 0x00, key_info.type);
    update(address + 0x01, key_info.key);
}
