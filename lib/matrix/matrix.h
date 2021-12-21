#pragma once

#include "dalsik.h"

class Matrix {
    private:
        uint8_t debounce_input(uint8_t row, uint8_t col, uint8_t input);

        uint8_t keystate[ROW_PIN_COUNT][ONE_SIDE_COL_PIN_COUNT];
        uint8_t debounce[ROW_PIN_COUNT][ONE_SIDE_COL_PIN_COUNT];
    public:
        Matrix();
        ChangedKeyEvent scan();
};
