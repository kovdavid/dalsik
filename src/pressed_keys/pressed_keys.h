#pragma once

#include "dalsik.h"
#include "key_event.h"
#include "pressed_key.h"

#define PRESSED_KEY_BUFFER 10

class PressedKeys {
    public:
        uint8_t count;
        millisec last_press;
        PressedKey keys[PRESSED_KEY_BUFFER];

        PressedKeys();
        PressedKey* add(KeyEvent event, uint8_t key_press_counter);
        PressedKey* find(KeyCoords coords);
        void remove(PressedKey* pk);

        inline bool is_empty() { return this->count == 0; }
        inline PressedKey* get(uint8_t index) { return &(this->keys[index]); }
        inline PressedKey* get_last() { return &(this->keys[this->count-1]); }
};
