#include "Arduino.h"
#include <avr/pgmspace.h>

#include "keymap.h"
#include "array_utils.h"
#include "dalsik.h"
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
    this->active_layer = 0;
    memset(this->activated_layers, 0, sizeof(this->activated_layers));
}

KeyInfo KeyMap::get_key(KeyCoords c) {
    KeyInfo key_info = this->get_key_from_layer(this->active_layer, c);

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
    for (int8_t i = ACTIVATED_LAYERS_CAPACITY-1; i >= 0; i--) {
        uint8_t layer = this->activated_layers[i];
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

#ifdef LED_PIN
    DalsikLED::set_rgb(LED_LAYER_COLORS[layer]);
#endif
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
