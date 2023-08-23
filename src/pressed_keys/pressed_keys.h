#pragma once

#include "dalsik.h"
#include "extended_key_event.h"
#include "pressed_key.h"

#define PRESSED_KEY_CAPACITY 10

class PressedKeys {
    public:
        uint8_t count;
        PressedKey keys[PRESSED_KEY_CAPACITY];

        PressedKeys();
        PressedKey* add(ExtendedKeyEvent event, uint8_t key_press_counter);
        PressedKey* find(KeyCoords coords);
        PressedKey* get_last();

        void remove(PressedKey* pk);
        void print_internal_state(millisec timestamp);
};
