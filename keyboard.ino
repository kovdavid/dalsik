#include "Arduino.h"
#include "HID.h"
#include "dalsik.h"
#include "dalsik_hid_desc.h"

extern const uint8_t KEYBOARD_HID_DESC[] PROGMEM;

Keyboard::Keyboard() {
    static HIDSubDescriptor node(KEYBOARD_HID_DESC, sizeof(KEYBOARD_HID_DESC));
    HID().AppendDescriptor(&node);

    memset(this->keystate, 0, sizeof(uint8_t)*ROW_PIN_COUNT*ONE_SIDE_COL_PIN_COUNT);
    memset(this->debounce, 0, sizeof(uint8_t)*ROW_PIN_COUNT*ONE_SIDE_COL_PIN_COUNT);

    for (uint8_t i = 0; i < ROW_PIN_COUNT; i++) {
        pinMode(ROW_PINS[i], INPUT_PULLUP);
    }
    for (uint8_t i = 0; i < ONE_SIDE_COL_PIN_COUNT; i++) {
        pinMode(COL_PINS[i], INPUT_PULLUP);
    }
}

ChangedKeyCoords Keyboard::matrix_scan() {
    for (uint8_t row = 0; row < ROW_PIN_COUNT; row++) {
        pinMode(ROW_PINS[row], OUTPUT);
        digitalWrite(ROW_PINS[row], LOW);

        for (uint8_t col = 0; col < ONE_SIDE_COL_PIN_COUNT; col++) {
            uint8_t input = !digitalRead(COL_PINS[col]);
            uint8_t debounced_input = this->debounce_input(row, col, input);

            if (debounced_input == DEBOUNCE_CHANGING) {
                continue; // Wait, till the value stabilizes
            }

            if (debounced_input != this->keystate[row][col]) {
                this->keystate[row][col] = debounced_input;
                pinMode(ROW_PINS[row], INPUT_PULLUP);

                if (debounced_input == DEBOUNCE_MAX) {
                    return ChangedKeyCoords { EVENT_KEY_PRESS, row, col };
                } else {
                    return ChangedKeyCoords { EVENT_KEY_RELEASE, row, col };
                }
            }
        }

        pinMode(ROW_PINS[row], INPUT_PULLUP);
    }

    return ChangedKeyCoords { EVENT_NONE, 0x00, 0x00 };
}

uint8_t Keyboard::debounce_input(uint8_t row, uint8_t col, uint8_t input) {
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
