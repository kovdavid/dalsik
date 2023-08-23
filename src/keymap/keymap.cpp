#include "Arduino.h"
#include <avr/pgmspace.h>

#include "keymap.h"
#include "array_utils.h"
#include "dalsik.h"
#include "key_definitions.h"
#include "key.h"

#define DEFAULT_LAYER 0

KeyMap::KeyMap()
    : activated_layers()
    , activated_layers_count(0)
{}

uint8_t KeyMap::active_layer() {
    if (this->activated_layers_count > 0) {
        return this->activated_layers[this->activated_layers_count-1];
    } else {
        return DEFAULT_LAYER;
    }
}

Key KeyMap::get_key(KeyCoords coords) {
    for (int8_t i = this->activated_layers_count; i > 0; i--) {
        uint8_t layer = this->activated_layers[i-1];

        Key key = this->get_key_from_layer(layer, coords);
        if (key.type != KEY_TRANSPARENT) {
            return key;
        }
    }

    Key key = this->get_key_from_layer(DEFAULT_LAYER, coords);
    if (key.type == KEY_TRANSPARENT) {
        return Key(KEY_NO_ACTION, coords);
    } else {
        return key;
    }
}

Key KeyMap::get_key_from_layer(uint8_t layer, KeyCoords coords) {
    if (
        layer < layer_count
        && coords.row <= KEYBOARD_ROWS
        && coords.col <= 2*KEYBOARD_COLS
    ) {
        uint32_t progmem_data = pgm_read_dword(&keymap[layer][coords.row][coords.col]);
        return Key(progmem_data, coords);
    }
    return Key(KC_NO, coords);
}

void KeyMap::layer_change_hook() {
#ifdef REPORT_LAYER_CHANGE
    Serial.print("L:");
    Serial.print(this->active_layer(), HEX);
    Serial.print("\n");
#endif
}

void KeyMap::activate_layer(uint8_t layer) {
    if (
        layer >= layer_count
        || this->activated_layers_count >= ACTIVATED_LAYERS_CAPACITY
    ) {
        // Invalid layer or too many activated layers
        return;
    }

    if (layer == this->active_layer()) {
        // Layer already active
        return;
    }

    this->activated_layers[this->activated_layers_count++] = layer;
    this->layer_change_hook();
}

void KeyMap::deactivate_layer(uint8_t layer) {
    int8_t found_index = -1;
    for (int8_t i = 0; i < this->activated_layers_count; i++) {
        if (this->activated_layers[i] == layer) {
            found_index = i;
            break;
        }
    }

    if (found_index == -1) {
        // The layer was not activated
        return;
    }

    // Shift elements to the left to fill the gap
    for (int8_t i = found_index; i < this->activated_layers_count - 1; i++) {
        this->activated_layers[i] = this->activated_layers[i+1];
    }

    this->activated_layers_count--;

    this->layer_change_hook();
}

void KeyMap::toggle_layer(uint8_t layer) {
    bool already_active = false;
    for (int8_t i = 0; i < this->activated_layers_count; i++) {
        if (this->activated_layers[i] == layer) {
            already_active = true;
            break;
        }
    }

    if (already_active) {
        this->deactivate_layer(layer);
    } else {
        this->activate_layer(layer);
    }
}

void KeyMap::print_internal_state() {
    Serial.print("KeyMap: active_layer:");
    Serial.print(this->active_layer());
    Serial.print(", activated_layers_count:");
    Serial.print(this->activated_layers_count);
    Serial.print(", activated_layers:");
    for (int8_t i = 0; i < this->activated_layers_count; i++) {
        Serial.print(this->activated_layers[i]);
        Serial.print("|");
    }
}
