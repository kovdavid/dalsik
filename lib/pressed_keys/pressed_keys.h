#pragma once

#include "dalsik.h"
#include "key_info.h"

#define STATE_NOT_PROCESSED   0x00
#define STATE_PENDING         0x01
#define STATE_ACTIVE_KEY      0x02
#define STATE_ACTIVE_MODIFIER 0x03
#define STATE_ACTIVE_LAYER    0x04
#define STATE_RELEASED        0x05

#define PRESSED_KEY_BUFFER 10

typedef struct {
    KeyInfo key_info;
    millisec timestamp;
    uint8_t key_press_counter;
    uint8_t state;
    uint8_t key_index;
} PressedKey;

class PressedKeys {
    public:
        uint8_t count;
        PressedKey keys[PRESSED_KEY_BUFFER];

        PressedKeys();
        PressedKey* add(KeyInfo key_info, millisec now, uint8_t key_press_counter);
        PressedKey* find(KeyCoords coords);
        void remove(PressedKey* pk);

        inline bool is_empty() { return this->count == 0; }
        inline PressedKey* get(uint8_t index) { return &(this->keys[index]); }
        inline PressedKey* get_last() { return &(this->keys[this->count-1]); }
};
