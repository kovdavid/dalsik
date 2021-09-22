# KeyMap

* [keymap.h](https://github.com/DavsX/dalsik/blob/master/keymap.h)
* [keymap.ino](https://github.com/DavsX/dalsik/blob/master/keymap.ino)

This module is responsible for accessing the keymap in the EEPROM, providing information about the keys and managing the layers.

# Keyboard side

The PCB of the two sides is identical (for financial reasons) and one of the sides is upside down compared to the way the column pins are mapped:

```c++
// dalsik.h
const uint8_t COL_PINS[ONE_SIDE_COL_PIN_COUNT] = {
    PIN_F(6), PIN_F(7), PIN_B(1), PIN_B(3), PIN_B(2), PIN_B(6)
};
```

This mapping is for the side, which has the USB connector on the left and the TRRS connector on the right. The other side has these pins inverted. Because of this we need to identify, which side is the code running on (note, that this sidedness is independent from the master-slave distinction).

This information has to be manually encoded into the keyboard (via the SerialCommand module) to the last byte of the EEPROM:

```c++
// keymap.h
#define KEYBOARD_SIDE_LEFT  0x00
#define KEYBOARD_SIDE_RIGHT 0x01

// keymap.ino
#define KEYBOARD_SIDE_EEPROM_ADDRESS EEPROM.length() - 1

void KeyMap::update_keyboard_side(uint8_t side) {
    if (side == KEYBOARD_SIDE_LEFT || side == KEYBOARD_SIDE_RIGHT) {
        EEPROM.update(KEYBOARD_SIDE_EEPROM_ADDRESS, side);
        this->keyboard_side = side;
    }
}
```



## KeyMap::get_key

One of the most often used functionality of this module is getting the KeyInfo struct for a given key by row/column coordinates.

```c++
// keymap.ino
KeyInfo KeyMap::get_key(uint8_t row, uint8_t col) {
    uint32_t eeprom_address = this->get_eeprom_address(this->layer_index, row, col);

    uint8_t type = EEPROM.read(eeprom_address);
    uint8_t key = EEPROM.read(eeprom_address + 0x01);

    KeyInfo key_info = KeyMap::init_key_info(type, key, row, col);
    if (key_info.type == KEY_TRANSPARENT) { // Get the key from lower layers
        key_info = this->get_non_transparent_key(row, col);
    }

    return key_info;
}
```

The function is not called directly. Instead, the `get_master_key` and `get_slave_key` wrappers are used. 

```c++
// keymap.ino

// The right side PCB of the Let's Split is reversed, so if it sends col 0, it is actually col 5.
// Also, the keyboard is represented in EEPROM as a whole, so the reported col must be incremented
// by ONE_SIDE_COL_PIN_COUNT (col 4 becomes col 10, as there are 6 columns per side)
// The right side sends columns 0-5 and thus we offset it to 6-11
KeyInfo KeyMap::get_master_key(uint8_t row, uint8_t col) {
    if (this->keyboard_side == KEYBOARD_SIDE_RIGHT) {
        col = 2*ONE_SIDE_COL_PIN_COUNT - col - 1;
    }
    return this->get_key(row, col);
}

KeyInfo KeyMap::get_slave_key(uint8_t row, uint8_t col) {
    // KeyMap is used only on the master side; The slave is the right side if the master is the left
    if (this->keyboard_side == KEYBOARD_SIDE_LEFT) {
        col = 2*ONE_SIDE_COL_PIN_COUNT - col - 1;
    }
    return this->get_key(row, col);
}
```

As the keymap in the EEPROM is represented as a keyboard with 4 rows and 12 columns, these wrapper methods provide the necessary adjustments of the column variable.

# Layer management

```c++
// keymap.h
#define LAYER_HISTORY_CAPACITY 5

class KeyMap {
    private:
        uint8_t layer_index;
        uint8_t toggled_layer_index;
        uint8_t layer_history[LAYER_HISTORY_CAPACITY];
    
    ...
}
```

The current layer is defined inside `this->layer_index` - this variable is used in `this->get_eeprom_address`.

If a layer is toggled, it is additionally defined inside `this->toggled_layer_index`. Using this variable we can set the correct layer after releasing every key for this layer instead of the default `0`.

It is possible to switch to a different layer from a non-default layer (e.g. 0 -> 2 -> 1). In this example after switching the layer from 0 to 2, then from 2 to 1 and releasing the `LAYER_PRESS` key for 1 the keyboard switches back to layer 2 as expected, instead of the default layer 0. This could not be done without the `layer_history` array.



When the keyboard is switched to a layer, the new layer index is appended to `layer_history`.

```c++
// array_utils.ino
void append_uniq_to_uint8_array(uint8_t* array, uint8_t array_size, uint8_t elem) {
    for (uint8_t i = 0; i < array_size-1; i++) {
        if (array[i] == elem) {
            return;
        }
        if (array[i] == 0x00) {
            array[i] = elem;
            return;
        }
    }
}

// keymap.ino
void KeyMap::set_layer(uint8_t layer) {
    this->layer_index = layer;
    append_uniq_to_uint8_array(this->layer_history, LAYER_HISTORY_CAPACITY, layer);
}
```



Similarly, when we switch off of the current layer, its index is removed from `layer_history` and the keyboard is switched to the previous layer.

```c++
// array_utils.ino

// Keep every non-zero element on the left after the removal
// Bad:  [0x01, 0x00, 0x03, 0x04, 0x00]
// Good: [0x01, 0x03, 0x04, 0x00, 0x00]
void remove_uniq_from_uint8_array(uint8_t* array, uint8_t array_size, uint8_t elem) {
    if (elem == 0x00) {
        return; // Nothing to do
    }

    uint8_t last_nonzero_elem_index = 0;
    for (uint8_t i = 0; i < array_size; i++) {
        if (array[i] == elem || array[i] == 0x00) {
            array[i] = 0x00;
            continue;
        }

        if (last_nonzero_elem_index == i) {
            array[last_nonzero_elem_index++] = array[i];
        } else {
            array[last_nonzero_elem_index++] = array[i];
            array[i] = 0x00;
        }
    }
}

uint8_t last_nonzero_elem_of_uint8_array(uint8_t* array, uint8_t array_size) {
    for (int8_t i = array_size-1; i >= 0; i--) {
        if (array[i] != 0x00) {
            return array[i];
        }
    }

    return 0x00;
}

// keymap.ino
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
```



Toggling a layer is done using `this->toggled_layer_index`

```c++
// keymap.ino
void KeyMap::toggle_layer(uint8_t layer) {
    if (this->toggled_layer_index == layer) {
        this->toggled_layer_index = 0;
        this->remove_layer(layer);
    } else {
        this->toggled_layer_index = layer;
        this->set_layer(layer);
    }
}
```



## Transparent keys

When a key is set to transparent, the KeyInfo of the same key from the previous layer is returned. This is done by traversing the `layer_history` variable backwards. The first non-transparent key for the given row/column coordinates is returned.

```c++
// keymap.ino
KeyInfo KeyMap::get_non_transparent_key(uint8_t row, uint8_t col) {
    for (int8_t i = LAYER_HISTORY_CAPACITY-1; i >= 0; i--) {
        uint8_t layer = this->layer_history[i];
        if (layer == 0x00) {
            continue;
        }

        KeyInfo key_info = this->get_key_from_layer(layer, row, col);
        if (key_info.type != KEY_TRANSPARENT) {
            return key_info;
        }
    }

    KeyInfo key_info = this->get_key_from_layer(0, row, col);
    if (key_info.type == KEY_TRANSPARENT) {
        return KeyMap::init_key_info(KEY_UNSET, 0x00, row, col);
    } else {
        return key_info;
    }
}

KeyInfo KeyMap::get_key_from_layer(uint8_t layer, uint8_t row, uint8_t col) {
    uint32_t eeprom_address = this->get_eeprom_address(layer, row, col);

    uint8_t type = EEPROM.read(eeprom_address);
    uint8_t key = EEPROM.read(eeprom_address + 0x01);

    return KeyMap::init_key_info(type, key, row, col);
}
```

## Setting a key

Setting a key is simply done by writing `KeyInfo.type` and `KeyInfo.key` bytes to the corresponding EEPROM address.

```c++
// keymap.ino
void KeyMap::set_key(uint8_t layer, KeyInfo key_info) {
    uint8_t row = key_info.row;
    uint8_t col = key_info.col;

    if (row == ROW_UNKNOWN || col == COL_UNKNOWN) {
        return; // Invalid KeyInfo
    }

    int eeprom_address = this->get_eeprom_address(layer, row, col);

    EEPROM.update(eeprom_address + 0x00, key_info.type);
    EEPROM.update(eeprom_address + 0x01, key_info.key);
}
```

