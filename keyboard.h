#ifndef KEYBOARD_h
#define KEYBOARD_h

#include "dalsik.h"
#include "keymap.h"

#define EVENT_NONE 0x00
#define EVENT_KEY_PRESS 0x01
#define EVENT_KEY_RELEASE 0x02

typedef struct {
    uint8_t type;
    KeyInfo key_info;
} KeyChangeEvent;

class Keyboard
{
    private:
        uint8_t debounce_input(uint8_t row, uint8_t col, uint8_t input);

        uint8_t keystate[ROW_PIN_COUNT][COL_PIN_COUNT];
        uint8_t debounce[ROW_PIN_COUNT][COL_PIN_COUNT];

        KeyMap* keymap;
    public:
        Keyboard(KeyMap* keymap);
        KeyChangeEvent matrix_scan();
};

#endif
