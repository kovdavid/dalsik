#include "Arduino.h"
#include <avr/pgmspace.h>

#include "keymap.h"
#include "array_utils.h"
#include "dalsik.h"
#include "key_definitions.h"
#include "key.h"

KeyMap::KeyMap()
    : active_layer(0)
    , activated_layers()
{}

Key KeyMap::get_key(KeyCoords coords) {
    Key key = this->get_key_from_layer(this->active_layer, coords);

    if (key.type == KEY_TRANSPARENT) { // Get the key from lower layers
        key = this->get_non_transparent_key(coords);
    }

    return key;
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

Key KeyMap::get_non_transparent_key(KeyCoords coords) {
    for (int8_t i = ACTIVATED_LAYERS_CAPACITY-1; i >= 0; i--) {
        uint8_t layer = this->activated_layers[i];
        if (layer == 0x00) {
            continue;
        }

        Key key = this->get_key_from_layer(layer, coords);
        if (key.type != KEY_TRANSPARENT) {
            return key;
        }
    }

    Key key = this->get_key_from_layer(0, coords);
    if (key.type == KEY_TRANSPARENT) {
        return Key(KEY_NO_ACTION, coords);
    } else {
        return key;
    }
}

void KeyMap::activate_layer(uint8_t layer) {
    if (this->active_layer == layer || layer >= layer_count) {
        return;
    }

#ifdef REPORT_LAYER_CHANGE
    Serial.print("L:");
    Serial.print(layer, HEX);
    Serial.print("\n");
#endif

    this->active_layer = layer;
    ArrayUtils::append_uniq_uint8(this->activated_layers, ACTIVATED_LAYERS_CAPACITY, layer);
}

void KeyMap::deactivate_layer(uint8_t layer) {
    uint8_t prev_layer = ArrayUtils::remove_and_return_last_uint8(
        this->activated_layers, ACTIVATED_LAYERS_CAPACITY, layer
    );

    this->activate_layer(prev_layer);
}

void KeyMap::toggle_layer(uint8_t layer) {
    bool already_active = ArrayUtils::contains_uint8(this->activated_layers, ACTIVATED_LAYERS_CAPACITY, layer);
    if (already_active) {
        this->deactivate_layer(layer);
    } else {
        this->activate_layer(layer);
    }
}

void KeyMap::print_internal_state() {
    Serial.print("KeyMap: active_layer:");
    Serial.print(active_layer);
    Serial.print(", activated_layers:");
    for (uint8_t i = 0; i < ACTIVATED_LAYERS_CAPACITY-1; i++) {
        uint8_t layer = this->activated_layers[i];
        if (layer == 0x00) {
            break;
        }
        Serial.print("|");
        Serial.print(layer);
    }
}
