#pragma once

#define E2END 1023

uint8_t eeprom_read_byte(uint8_t* addr);
void eeprom_write_byte(uint8_t* addr, uint8_t value);
