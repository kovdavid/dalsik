#ifndef MATRIX_H
#define MATRIX_H

#include "dalsik.h"

// Cherry claims bouncing to be <5ms
#define DEBOUNCE_MAX 0x05
#define DEBOUNCE_LOW 0x00
#define DEBOUNCE_CHANGING 0xFF

#define EVENT_NONE 0x00
#define EVENT_KEY_PRESS 0x01
#define EVENT_KEY_RELEASE 0x02

typedef struct {
    uint8_t type;
    uint8_t row;
    uint8_t col;
} ChangedKeyCoords;

inline static void pinmode_input_pullup(uint8_t pin);
inline static void pinmode_output_low(uint8_t pin);
inline static uint8_t read_pin(uint8_t pin);

class Matrix {
    private:
        uint8_t debounce_input(uint8_t row, uint8_t col, uint8_t input);

        uint8_t keystate[ROW_PIN_COUNT][ONE_SIDE_COL_PIN_COUNT];
        uint8_t debounce[ROW_PIN_COUNT][ONE_SIDE_COL_PIN_COUNT];
    public:
        Matrix();
        ChangedKeyCoords scan();
};

#endif
