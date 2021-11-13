#include <string.h>
#include "dalsik.h"
#include "dalsik_eeprom.h"
#include "keymap.h"
#include "array_utils.h"
#include "dalsik_led.h"

KeyMap::KeyMap() {
    this->layer_index = 0;
    this->toggled_layer_index = 0;
    this->keyboard_side = EEPROM::get_keyboard_side();
    this->clear();
}

void KeyMap::clear() {
    this->layer_index = 0;
    memset(this->layer_history, 0, sizeof(uint8_t)*LAYER_HISTORY_CAPACITY);
    if (this->toggled_layer_index > 0) {
        this->set_layer(this->toggled_layer_index);
    }
}

// The right side PCB of the Let's Split is reversed, so if it sends col 0, it is actually col 5.
// Also, the keyboard is represented in EEPROM as a whole, so the reported col must be incremented
// by ONE_SIDE_COL_PIN_COUNT (col 4 becomes col 10, as there are 6 columns per side)
// The right side sends columns 0-5 and thus we offset it to 6-11
KeyInfo KeyMap::get_master_key(KeyCoords c) {
    if (this->keyboard_side == KEYBOARD_SIDE_RIGHT) {
        c.col = 2*ONE_SIDE_COL_PIN_COUNT - c.col - 1;
    }
    return this->get_key(c);
}

KeyInfo KeyMap::get_slave_key(KeyCoords c) {
    // KeyMap is used only on the master side; The slave is the right side if the master is the left
    if (this->keyboard_side == KEYBOARD_SIDE_LEFT) {
        c.col = 2*ONE_SIDE_COL_PIN_COUNT - c.col - 1;
    }
    return this->get_key(c);
}

KeyInfo KeyMap::get_key(KeyCoords c) {
    KeyInfo key_info = EEPROM::get_key(this->layer_index, c);

    if (key_info.type == KEY_TRANSPARENT) { // Get the key from lower layers
        key_info = this->get_non_transparent_key(c);
    }

    return key_info;
}

void KeyMap::reload_by_coords(KeyInfo* ki) {
    if (ki->has_no_coords()) {
        return; // Missing coords info
    }

    KeyInfo new_ki = this->get_key(ki->coords);
    ki->type = new_ki.type;
    ki->key = new_ki.key;
}

void KeyMap::set_layer(uint8_t layer) {
    this->layer_index = layer;
    append_uniq_to_uint8_array(this->layer_history, LAYER_HISTORY_CAPACITY, layer);
#ifdef LED_PIN
    set_led_rgb(LED_LAYER_COLORS[layer]);
#endif
}

// We could have pressed multiple layer keys, so releasing one means we switch to the other one
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

KeyInfo KeyMap::get_non_transparent_key(KeyCoords c) {
    for (int8_t i = LAYER_HISTORY_CAPACITY-1; i >= 0; i--) {
        uint8_t layer = this->layer_history[i];
        if (layer == 0x00) {
            continue;
        }

        KeyInfo key_info = EEPROM::get_key(layer, c);
        if (key_info.type != KEY_TRANSPARENT) {
            return key_info;
        }
    }

    KeyInfo key_info = EEPROM::get_key(0, c);
    if (key_info.type == KEY_TRANSPARENT) {
        return KeyInfo(KEY_UNSET, 0x00, c);
    } else {
        return key_info;
    }
}
