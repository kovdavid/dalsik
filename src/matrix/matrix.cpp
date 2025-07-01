#include <string.h>
#include "matrix.h"
#include "dalsik.h"
#include "pin_utils.h"

// Cherry claims bouncing to be <5ms
#define DEBOUNCE_MAX 0x05
#define DEBOUNCE_MIN 0x00
#define DEBOUNCE_CHANGING 0xFF

Matrix::Matrix() {
    memset(this->keystate, 0, sizeof(uint8_t)*KEYBOARD_ROWS*KEYBOARD_COLS);
    memset(this->debounce, 0, sizeof(uint8_t)*KEYBOARD_ROWS*KEYBOARD_COLS);

    this->init();
}

#ifdef FERRIS_SWEEP
void Matrix::init() {
    for (uint8_t row = 0; row < KEYBOARD_ROWS; row++) {
        for (uint8_t col = 0; col < KEYBOARD_COLS; col++) {
            uint8_t pin = DIRECT_PINS[row][col];
            if (!pin) {
                continue;
            }

            PinUtils::pinmode_input_pullup(pin);
        }
    }
}

BaseKeyEvent Matrix::scan() {
    for (uint8_t row = 0; row < KEYBOARD_ROWS; row++) {
        for (uint8_t col = 0; col < KEYBOARD_COLS; col++) {
            uint8_t pin = DIRECT_PINS[row][col];
            if (!pin) {
                continue;
            }

            uint8_t input = !PinUtils::read_pin(pin);
            uint8_t debounced_input = this->debounce_input(row, col, input);

            if (debounced_input == DEBOUNCE_CHANGING) {
                continue; // Wait, till the value stabilizes
            }

            if (debounced_input != this->keystate[row][col]) {
                this->keystate[row][col] = debounced_input;

                KeyCoords coords = { row, col };

                if (debounced_input == DEBOUNCE_MAX) {
                    return BaseKeyEvent { EVENT_KEY_PRESS, coords };
                } else {
                    return BaseKeyEvent { EVENT_KEY_RELEASE, coords };
                }
            }
        }
    }

    return BaseKeyEvent { EVENT_NONE, KeyCoords { 0x00, 0x00 } };
}
#else
void Matrix::init() {
   for (uint8_t row = 0; row < KEYBOARD_ROWS; row++) {
        PinUtils::pinmode_input_pullup(ROW_PINS[row]);
    }
    for (uint8_t col = 0; col < KEYBOARD_COLS; col++) {
        PinUtils::pinmode_input_pullup(COL_PINS[col]);
    }
}

BaseKeyEvent Matrix::scan() {
    for (uint8_t row = 0; row < KEYBOARD_ROWS; row++) {
        PinUtils::pinmode_output_low(ROW_PINS[row]);

        for (uint8_t col = 0; col < KEYBOARD_COLS; col++) {
            uint8_t input = !PinUtils::read_pin(COL_PINS[col]);
            uint8_t debounced_input = this->debounce_input(row, col, input);

            if (debounced_input == DEBOUNCE_CHANGING) {
                continue; // Wait, till the value stabilizes
            }

            if (debounced_input != this->keystate[row][col]) {
                this->keystate[row][col] = debounced_input;
                PinUtils::pinmode_input_pullup(ROW_PINS[row]);

                KeyCoords coords = { row, col };

                if (debounced_input == DEBOUNCE_MAX) {
                    return BaseKeyEvent { EVENT_KEY_PRESS, coords };
                } else {
                    return BaseKeyEvent { EVENT_KEY_RELEASE, coords };
                }
            }
        }

        PinUtils::pinmode_input_pullup(ROW_PINS[row]);
    }

    return BaseKeyEvent { EVENT_NONE, KeyCoords { 0x00, 0x00 } };
}
#endif


uint8_t Matrix::debounce_input(uint8_t row, uint8_t col, uint8_t input) {
    if (input) {
        if (this->debounce[row][col] < DEBOUNCE_MAX) {
            this->debounce[row][col]++;
        }
        if (this->debounce[row][col] == DEBOUNCE_MAX) {
            return DEBOUNCE_MAX;
        }
    } else {
        if (this->debounce[row][col] > DEBOUNCE_MIN) {
            this->debounce[row][col]--;
        }
        if (this->debounce[row][col] == DEBOUNCE_MIN) {
            return DEBOUNCE_MIN;
        }
    }
    return DEBOUNCE_CHANGING;
}
