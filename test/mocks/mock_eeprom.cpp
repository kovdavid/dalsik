#include "avr/eeprom.h"
#include "dalsik.h"
#include "dalsik_eeprom.h"
#include "key_definitions.h"

#define SET_KEY(layer, row, col, type, key) \
    data[EEPROM::get_eeprom_address(layer, KeyCoords { row, col }) + 0] = type; \
    data[EEPROM::get_eeprom_address(layer, KeyCoords { row, col }) + 1] = key;

static uint8_t* get_eeprom_data() {
    uint8_t* data = (uint8_t*) calloc(E2END+1, sizeof(uint8_t));

    SET_KEY(0, 1, 1, KEY_NORMAL, KC_A);
    SET_KEY(0, 1, 2, KEY_NORMAL, KC_B);
    SET_KEY(0, 1, 3, KEY_DUAL_MOD_LCTRL, KC_C);
    SET_KEY(0, 1, 4, KEY_DUAL_MOD_LSHIFT, KC_D);
    SET_KEY(0, 1, 5, KEY_TIMED_DUAL_MOD_LSHIFT, KC_E);
    SET_KEY(0, 1, 6, KEY_TIMED_DUAL_MOD_LCTRL, KC_F);

    return data;
}

uint8_t* data = get_eeprom_data();


uint8_t eeprom_read_byte(uint8_t* addr) {
    return *(data + (size_t)addr);
}

void eeprom_write_byte(uint8_t* addr, uint8_t value) {
    data[(size_t)addr] = value;
}
