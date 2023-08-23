#include "Arduino.h"
#include "combo.h"
#include "bit_utils.h"
#include <avr/pgmspace.h>

#ifndef COMBOS_ENABLED
Combo combos[] = { Combo(NULL, KC_NO, 0) };
const uint8_t combos_count = 0;
#endif

Combo::Combo(const KeyCoords* coords, const uint32_t target_key, const uint8_t coords_count)
    : coords(coords)
    , target_key(target_key)
    , coords_count(coords_count)
    , pressed_coords(0x00)
    , flags(0x00)
{}

KeyCoords Combo::read_coords_at(uint8_t index) {
    KeyCoords c;
    memcpy_P(&c, &(this->coords[index]), sizeof(KeyCoords));
    return c;
}

bool Combo::update_state_if_affected(KeyCoords param_coords) {
    bool affected = false;

    for (uint8_t i = 0; i < this->coords_count; i++) {
        KeyCoords combo_coords = this->read_coords_at(i);
        if (combo_coords.equals(param_coords)) {
            affected = true;
            BIT_SET(this->pressed_coords, 1 << i);
        }
    }

    return affected;
}

void Combo::release_coords(KeyCoords param_coords) {
    for (uint8_t i = 0; i < this->coords_count; i++) {
        KeyCoords combo_coords = this->read_coords_at(i);
        if (combo_coords.equals(param_coords)) {
            BIT_CLEAR(this->pressed_coords, 1 << i);
        }
    }
}

void Combo::set_activated_flag() {
    BIT_SET(this->flags, FLAG_ACTIVATED);
}

void Combo::set_disabled_flag() {
    BIT_SET(this->flags, FLAG_DISABLED);
}

void Combo::clear_disabled_flag() {
    BIT_CLEAR(this->flags, FLAG_DISABLED);
}

void Combo::clear_state() {
    this->pressed_coords = 0x00;
    this->flags = 0x00;
}

bool Combo::is_activated() {
    return this->flags & FLAG_ACTIVATED;
}

bool Combo::is_disabled() {
    return this->flags & FLAG_DISABLED;
}

bool Combo::is_fully_pressed() {
    return (this->pressed_coords == (1 << this->coords_count) - 1);
}

bool Combo::is_fully_released() {
    return this->pressed_coords == 0;
}

void Combo::print_internal_state(uint8_t index) {
    Serial.print("Combo #");
    Serial.print(index);
    Serial.print(" coords_count:");
    Serial.print(this->coords_count);
    Serial.print(" target_key:");
    Serial.print(this->target_key, HEX);
    Serial.print(" pressed_coords:");
    Serial.print(this->pressed_coords, BIN);
    Serial.print(" flags:");
    Serial.print(this->flags, BIN);
    Serial.print(" keys:<");
    for (uint8_t i = 0; i < this->coords_count; i++) {
        KeyCoords coords = this->read_coords_at(i);
        Serial.print("R:");
        Serial.print(coords.row);
        Serial.print(",C:");
        Serial.print(coords.col);
        if (i < this->coords_count - 1) {
            Serial.print(" ");
        }
    }
    Serial.print(">\n");
}
