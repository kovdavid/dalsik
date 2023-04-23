#include "Arduino.h"
#include <avr/pgmspace.h>

#include "keymap.h"
#include "array_utils.h"
#include "dalsik_global.h"
#include "dalsik_led.h"
#include "key_definitions.h"
#include "key_info.h"

#ifdef LED_PIN
const uint32_t LED_LAYER_COLORS[MAX_LAYER_COUNT] = {
    0x00000000, // Layer 0 - default (LED off)
    0x00FF0000,
    0x0000FF00,
    0x000000FF,
    0x00FFFF00,
    0x00FF00FF,
    0x00000000,
    0x00000000,
};
#endif

KeyMap::KeyMap() {
    this->layer_index = 0;
    this->toggled_layer_index = 0;
    memset(this->layer_history, 0, sizeof(this->layer_history));
}

KeyInfo KeyMap::get_key(KeyCoords c) {
    KeyInfo key_info = this->get_key_from_layer(this->layer_index, c);

    if (key_info.type == KEY_TRANSPARENT) { // Get the key from lower layers
        key_info = this->get_non_transparent_key(c);
    }

    return key_info;
}

KeyInfo KeyMap::get_key_from_layer(uint8_t layer, KeyCoords c) {
    uint32_t progmem_data = pgm_read_dword(&keymap[layer][c.row][c.col]);
    return KeyInfo(progmem_data, c);
}

KeyInfo KeyMap::get_non_transparent_key(KeyCoords c) {
    for (int8_t i = LAYER_HISTORY_CAPACITY-1; i >= 0; i--) {
        uint8_t layer = this->layer_history[i];
        if (layer == 0x00) {
            continue;
        }

        KeyInfo key_info = this->get_key_from_layer(layer, c);
        if (key_info.type != KEY_TRANSPARENT) {
            return key_info;
        }
    }

    KeyInfo key_info = this->get_key_from_layer(0, c);
    if (key_info.type == KEY_TRANSPARENT) {
        return KeyInfo(KEY_NO_ACTION, c);
    } else {
        return key_info;
    }
}

void KeyMap::set_layer(uint8_t layer) {
    if (this->layer_index == layer || layer >= layer_count) {
        return;
    }

#ifdef REPORT_LAYER_CHANGE
    Serial.print("L:");
    Serial.print(layer, HEX);
    Serial.print("\n");
#endif

    this->layer_index = layer;
    append_uniq_to_uint8_array(this->layer_history, LAYER_HISTORY_CAPACITY, layer);

#ifdef LED_PIN
    set_led_rgb(LED_LAYER_COLORS[layer]);
#endif
}

void KeyMap::remove_layer(uint8_t layer) {
    remove_uniq_from_uint8_array(this->layer_history, LAYER_HISTORY_CAPACITY, layer);
    uint8_t prev_layer = last_nonzero_elem_of_uint8_array(
        this->layer_history, LAYER_HISTORY_CAPACITY
    );

    if (prev_layer > 0) {
        this->set_layer(prev_layer);
    } else {
        this->set_layer(this->toggled_layer_index); // toggled layer or 0
    }
}

void KeyMap::toggle_layer(uint8_t layer) {
    if (this->toggled_layer_index == layer) {
        this->toggled_layer_index = 0;
        this->remove_layer(layer);
    } else {
        this->toggled_layer_index = layer;
        this->set_layer(layer);
    }
}

void KeyMap::print_internal_state() {
    Serial.print("KeyMap: layer_index:");
    Serial.print(layer_index);
    Serial.print(", toggled_layer_index:");
    Serial.print(toggled_layer_index);
    Serial.print(", layer_history:");
    for (uint8_t i = 0; i < LAYER_HISTORY_CAPACITY-1; i++) {
        uint8_t layer = this->layer_history[i];
        if (layer == 0x00) {
            break;
        }
        Serial.print("|");
        Serial.print(layer);
    }
}
