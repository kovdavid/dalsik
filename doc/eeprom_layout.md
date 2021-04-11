# EEPROM layout

* [key_definitions.h](https://github.com/DavsX/dalsik/blob/master/key_definitions.h)
* [keymap.h](https://github.com/DavsX/dalsik/blob/master/keymap.h)
* [keymap.ino](https://github.com/DavsX/dalsik/blob/master/keymap.ino)

The EEPROM is non-volatile memory, with relatively slow write speed and limited number of supported writes. This makes it a good place to store the keymap of the keyboard.

```c++
// keymap.h

typedef struct {
    uint8_t type;
    uint8_t key;
} EEPROM_KeyInfo;
```

