#include "Arduino.h"
#include "combo_state.h"
#include "bit_utils.h"
#include <avr/pgmspace.h>

ComboState::ComboState(const KeyCoords* coords, uint32_t target_key, uint8_t coords_count) {
    this->coords = coords;
    this->target_key = target_key;
    this->coords_count = coords_count;
    this->timestamp = 0;
}

KeyCoords ComboState::read_coords_at(uint8_t index) {
    KeyCoords c;
    memcpy_P(&c, &(this->coords[index]), sizeof(KeyCoords));
    return c;
}

bool ComboState::update_state_if_affected(KeyCoords param_coords, millisec now) {
    bool affected = false;

    for (uint8_t i = 0; i < this->coords_count; i++) {
        KeyCoords combo_coords = this->read_coords_at(i);
        if (combo_coords.equals(param_coords)) {
            affected = true;
            if (this->is_fully_released()) { // First pressed key
                this->timestamp = now;
            }
            BIT_SET(this->state, 1 << i);
        }
    }

    return affected;
}

void ComboState::remove_coords_from_state(KeyCoords param_coords) {
    for (uint8_t i = 0; i < this->coords_count; i++) {
        KeyCoords combo_coords = this->read_coords_at(i);
        if (combo_coords.equals(param_coords)) {
            BIT_CLEAR(this->state, 1 << i);
        }
    }
}

void ComboState::set_activated() {
    BIT_SET(this->flags, FLAG_ACTIVATED);
}
void ComboState::set_disabled() {
    BIT_SET(this->flags, FLAG_DISABLED);
}
void ComboState::clear_state_keep_flags() {
    this->state = 0x00;
    this->timestamp = 0;
}
void ComboState::clear_disabled() {
    BIT_CLEAR(this->flags, FLAG_DISABLED);
}
void ComboState::clear_state_and_flags() {
    this->state = 0x00;
    this->flags = 0x00;
    this->timestamp = 0;
}
bool ComboState::is_activated() {
    return this->flags & FLAG_ACTIVATED;
}
bool ComboState::is_disabled() {
    return this->flags & FLAG_DISABLED;
}
bool ComboState::is_fully_pressed() {
    return (this->state == (1 << this->coords_count) - 1);
}
bool ComboState::is_fully_released() {
    return this->state == 0;
}

void ComboState::print_internal_state(uint8_t index, millisec now) {
    Serial.print("ComboState #");
    Serial.print(index);
    Serial.print(" coords_count:");
    Serial.print(this->coords_count);
    Serial.print(" target_key:");
    Serial.print(this->target_key, HEX);
    Serial.print(" state:");
    Serial.print(this->state, BIN);
    Serial.print(" flags:");
    Serial.print(this->flags, BIN);
    Serial.print(" timestamp:");
    Serial.print(this->timestamp);
    Serial.print(" now-timestamp:");
    Serial.print(now - this->timestamp);
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
