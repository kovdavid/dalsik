#include "avr/eeprom.h"
#include "dalsik.h"
#include "dalsik_eeprom.h"
#include "key_definitions.h"

static uint8_t* get_eeprom_data() {
    return (uint8_t*) calloc(E2END+1, sizeof(uint8_t));
}

uint8_t* data = get_eeprom_data();

uint8_t eeprom_read_byte(uint8_t* addr) {
    return *(data + (size_t)addr);
}

void eeprom_write_byte(uint8_t* addr, uint8_t value) {
    data[(size_t)addr] = value;
}
