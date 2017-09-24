#include "EEPROM.h"
#include "dalsik.h"

Keymap::Keymap() {
    uint8_t offset = EEPROM.read(0x00);

    // If there are no meta data, the keymap starts at 0x01
    if (offset != 0x01) {

    }

    this->eeprom_offset = offset;
}
