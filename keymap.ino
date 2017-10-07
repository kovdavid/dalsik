#include "EEPROM.h"
#include "dalsik.h"
#include "keymap.h"

KeyMap::KeyMap()
{
    uint8_t offset = EEPROM.read(0x00);
    this->eeprom_offset = offset;
    this->layer_index = 0;
}

void KeyMap::set_layer(uint8_t layer)
{
    this->layer_index = layer;
}

inline int KeyMap::get_eeprom_address(uint8_t row, uint8_t col)
{
    return
        this->eeprom_offset
        + this->layer_index*KEY_COUNT*sizeof(KeyInfo)
        + row*COL_PIN_COUNT
        + col;
}

KeyInfo KeyMap::get_key(uint8_t row, uint8_t col)
{
    int eeprom_address = this->get_eeprom_address(row, col);

    uint8_t b1 = EEPROM.read(eeprom_address + 0x00);
    uint8_t b2 = EEPROM.read(eeprom_address + 0x01);
    uint8_t b3 = EEPROM.read(eeprom_address + 0x02);

    KeyInfo key_info = { b1, b2, b3 };

    return key_info;
}

void KeyMap::set_key(uint8_t layer, uint8_t row, uint8_t col, KeyInfo key)
{
    int eeprom_address = this->get_eeprom_address(row, col);

    EEPROM.write(eeprom_address + 0x00, key.type);
    EEPROM.write(eeprom_address + 0x01, key.generic_bytes.byte1);
    EEPROM.write(eeprom_address + 0x02, key.generic_bytes.byte2);
}

void KeyMap::clear()
{
    EEPROM.write(0, 0x01); // eeprom_offset
    for (uint8_t i = 1; i < EEPROM.length(); i++) {
        EEPROM.write(i, 0x00);
    }
}
