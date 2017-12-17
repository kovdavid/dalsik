#include "dalsik.h"
#include "pin_utils.h"

Matrix::Matrix() {
    memset(this->keystate, 0, sizeof(uint8_t)*ROW_PIN_COUNT*ONE_SIDE_COL_PIN_COUNT);
    memset(this->debounce, 0, sizeof(uint8_t)*ROW_PIN_COUNT*ONE_SIDE_COL_PIN_COUNT);

    for (uint8_t row = 0; row < ROW_PIN_COUNT; row++) {
        PinUtils::pinmode_input_pullup(ROW_PINS[row]);
    }
    for (uint8_t col = 0; col < ONE_SIDE_COL_PIN_COUNT; col++) {
        PinUtils::pinmode_input_pullup(COL_PINS[col]);
    }
}

// duration: 168us when no change is detected
ChangedKeyCoords Matrix::scan() {
    for (uint8_t row = 0; row < ROW_PIN_COUNT; row++) {
        PinUtils::pinmode_output_low(ROW_PINS[row]);

        for (uint8_t col = 0; col < ONE_SIDE_COL_PIN_COUNT; col++) {
            uint8_t input = !PinUtils::read_pin(COL_PINS[col]);
            uint8_t debounced_input = this->debounce_input(row, col, input);

            if (debounced_input == DEBOUNCE_CHANGING) {
                continue; // Wait, till the value stabilizes
            }

            if (debounced_input != this->keystate[row][col]) {
                this->keystate[row][col] = debounced_input;
                PinUtils::pinmode_input_pullup(ROW_PINS[row]);

                if (debounced_input == DEBOUNCE_MAX) {
                    return ChangedKeyCoords { EVENT_KEY_PRESS, row, col };
                } else {
                    return ChangedKeyCoords { EVENT_KEY_RELEASE, row, col };
                }
            }
        }

        PinUtils::pinmode_input_pullup(ROW_PINS[row]);
    }

    return ChangedKeyCoords { EVENT_NONE, 0x00, 0x00 };
}

uint8_t Matrix::debounce_input(uint8_t row, uint8_t col, uint8_t input) {
    if (input) {
        if (this->debounce[row][col] < DEBOUNCE_MAX) {
            this->debounce[row][col]++;
        }
        if (this->debounce[row][col] == DEBOUNCE_MAX) {
            return DEBOUNCE_MAX;
        }
    } else {
        if (this->debounce[row][col] > 0) {
            this->debounce[row][col]--;
        }
        if (this->debounce[row][col] == 0) {
            return DEBOUNCE_LOW;
        }
    }
    return DEBOUNCE_CHANGING;
}
