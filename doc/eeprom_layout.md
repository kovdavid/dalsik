# EEPROM layout

* [key_definitions.h](https://github.com/DavsX/dalsik/blob/master/key_definitions.h)
* [keymap.h](https://github.com/DavsX/dalsik/blob/master/keymap.h)
* [keymap.ino](https://github.com/DavsX/dalsik/blob/master/keymap.ino)

The EEPROM is non-volatile memory, with relatively slow write speed and limited number of supported writes. This makes it a good place to store the keymap of the keyboard. As we can see, each KeyInfo struct is 2 bytes long:

```c++
// keymap.h

typedef struct {
    uint8_t type;
    uint8_t key;
} EEPROM_KeyInfo;
```

The keymap of the keyboard is stored in the EEPROM of the master side as a 4x12 grid (as if the keyboard were one piece). Each layer is therefor of a constant size of 4x12x2B = 96B. To get the base address of the KeyInfo struct of a given key (row+colum) on a given layer is as follows:

```c++
// dalsik.h
#define ROW_PIN_COUNT 4                                  
#define ONE_SIDE_COL_PIN_COUNT 6

// keymap.h
#define KEY_COUNT ROW_PIN_COUNT * 2 * ONE_SIDE_COL_PIN_COUNT

// keymap.ino
inline uint32_t KeyMap::get_eeprom_address(uint8_t layer, uint8_t row, uint8_t col) {
    return sizeof(EEPROM_KeyInfo)*( layer*KEY_COUNT + row*2*ONE_SIDE_COL_PIN_COUNT + col );
}
```

Dalsik supports 8 layers, so the first 8 *`KEY_COUNT` * `sizeof(EEPROM_KeyInfo)` = 768B are used to store the keymap for every layer.

TapDance keys are stored after the keymaps:

```c++
// keymap.h
#define MAX_LAYER_COUNT 8
#define TAPDANCE_EEPROM_OFFSET (sizeof(EEPROM_KeyInfo)*MAX_LAYER_COUNT*KEY_COUNT)
```

Each TapDance key is stored in a 3*`sizeof(EEPROM_KeyInfo)` = 6B block. The base address for the KeyInfo struct of a given TapDance index and number of taps is calculated like this:

```c++
// keymap.h
#define MAX_TAPDANCE_TAPS 3

// keymap.ino

inline uint32_t KeyMap::get_tapdance_eeprom_address(uint8_t index, uint8_t tap) {
    return TAPDANCE_EEPROM_OFFSET + sizeof(EEPROM_KeyInfo)*( index*MAX_TAPDANCE_TAPS + tap-1 );
}
```

