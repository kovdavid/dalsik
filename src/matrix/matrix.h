#pragma once

#include "pin_utils.h"
#include "keyboard_hardware.h"

#define EVENT_NONE 0x00
#define EVENT_KEY_PRESS 0x01
#define EVENT_KEY_RELEASE 0x02
#define EVENT_TIMEOUT 0x03

#define COORD_UNKNOWN 255
#define COORD_COMBO 127
#define COORDS_INVALID KeyCoords { COORD_UNKNOWN, COORD_UNKNOWN }

class KeyCoords {
    public:
        uint8_t row;
        uint8_t col;

        bool equals(KeyCoords other) {
            return this->row == other.row && this->col == other.col;
        }
};

typedef struct {
    uint8_t type;
    KeyCoords coords;
} BasicKeyEvent;

class Matrix {
    private:
        uint8_t debounce_input(uint8_t row, uint8_t col, uint8_t input);

        uint8_t keystate[ROW_PIN_COUNT][ONE_SIDE_COL_PIN_COUNT];
        uint8_t debounce[ROW_PIN_COUNT][ONE_SIDE_COL_PIN_COUNT];
    public:
        Matrix();
        BasicKeyEvent scan();
};
