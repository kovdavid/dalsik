#include "Arduino.h"
#include "HID.h"
#include "dalsik.h"
#include "keyboard_hid_desc.h"
#include "keyreport.h"

#define DEBOUNCE_MAX 5
#define DEBOUNCE_LOW 0x00
#define DEBOUNCE_CHANGING 0x02

extern const uint8_t KEYBOARD_HID_DESC[] PROGMEM;

Keyboard::Keyboard() {
    static HIDSubDescriptor node(KEYBOARD_HID_DESC, sizeof(KEYBOARD_HID_DESC));
    HID().AppendDescriptor(&node);

    memset(this->keystate, 0, sizeof(uint8_t)*ROW_PIN_COUNT*COL_PIN_COUNT);
    memset(this->debounce, 0, sizeof(uint8_t)*ROW_PIN_COUNT*COL_PIN_COUNT);

    for (uint8_t i = 0; i < ROW_PIN_COUNT; i++) {
        pinMode(ROW_PINS[i], INPUT_PULLUP);
    }
    for (uint8_t i = 0; i < COL_PIN_COUNT; i++) {
        pinMode(COL_PINS[i], INPUT_PULLUP);
    }
}

void Keyboard::loop(unsigned long now_msec)
{
    this->keyreport.check_special_keys(now_msec);
    this->matrix_scan(now_msec);
}

void Keyboard::matrix_scan(unsigned long now_msec)
{
    for (int row = 0; row < ROW_PIN_COUNT; row++) {
        pinMode(ROW_PINS[row], OUTPUT);
        digitalWrite(ROW_PINS[row], LOW);

        for (int col = 0; col < COL_PIN_COUNT; col++) {
            uint8_t input = !digitalRead(COL_PINS[col]);
            uint8_t debounced_input = this->debounce_input(row, col, input);

            if (debounced_input == DEBOUNCE_CHANGING) {
                continue; // Wait, till the value stabilizes
            }

            if (debounced_input != this->keystate[row][col]) {
                this->keystate[row][col] = debounced_input;

                if (debounced_input == DEBOUNCE_MAX) {
                    this->keyreport.press(now_msec, row, col);
                } else {
                    this->keyreport.release(now_msec, row, col);
                }

                pinMode(ROW_PINS[row], INPUT_PULLUP);
                return;
            }
        }

        pinMode(ROW_PINS[row], INPUT_PULLUP);
    }
}

uint8_t Keyboard::debounce_input(uint8_t row, uint8_t col, uint8_t input)
{
    uint8_t debounce_state = this->debounce[row][col];
    if (input) {
        if (debounce_state < DEBOUNCE_MAX) {
            this->debounce[row][col]++;
        }
        if (debounce_state == DEBOUNCE_MAX) {
            return DEBOUNCE_MAX;
        }
    } else {
        if (debounce_state > 0) {
            this->debounce[row][col]--;
        }
        if (debounce_state == 0) {
            return DEBOUNCE_LOW;
        }
    }

    return DEBOUNCE_CHANGING;
}
