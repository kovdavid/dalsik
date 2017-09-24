#include "Arduino.h"
#include "HID.h"
#include "dalsik.h"
#include "keyboard_hid_desc.h"
#include "keymap.h"

#define DEBOUNCE_MAX 5
#define DEBOUNCE_LOW 0x00
#define DEBOUNCE_HIGH 0x01
#define DEBOUNCE_CHANGING 0x02

extern const uint8_t KEYBOARD_HID_DESC[] PROGMEM;

uint8_t keystate[ROW_PIN_COUNT][COL_PIN_COUNT] = {0};
uint8_t debounce[ROW_PIN_COUNT][COL_PIN_COUNT] = {0};
KeyReport report;

Keyboard::Keyboard() {
    static HIDSubDescriptor node(KEYBOARD_HID_DESC, sizeof(KEYBOARD_HID_DESC));
    HID().AppendDescriptor(&node);

    for (uint8_t i = 0; i < ROW_PIN_COUNT; i++) {
        pinMode(ROW_PINS[i], INPUT_PULLUP);
    }
    for (uint8_t i = 0; i < COL_PIN_COUNT; i++) {
        pinMode(COL_PINS[i], INPUT_PULLUP);
    }

    this->keys_pressed = 0;
}

void Keyboard::loop(unsigned long now_msec)
{
    this->matrix_scan(now_msec);
}

void Keyboard::send_report(KeyReport* report) {
    HID().SendReport(KEYBOARD_REPORT_ID, report, sizeof(KeyReport));
}

void Keyboard::matrix_scan(unsigned long now_msec)
{
    for (int row = 0; row < ROW_PIN_COUNT; row++) {
        pinMode(ROW_PINS[row], OUTPUT);
        digitalWrite(ROW_PINS[row], LOW);

        for (int col = 0; col < COL_PIN_COUNT; col++) {
            int input = digitalRead(COL_PINS[col]);
            uint8_t debounced_input = debounce_input(&(debounce[row][col]), input);

            if (debounced_input == DEBOUNCE_CHANGING) {
                continue; // Wait, till the value stabilizes
            }

            if (debounced_input != keystate[row][col]) {
                keystate[row][col] = debounced_input;
                if (debounced_input == DEBOUNCE_MAX) {
                    this->keys_pressed++;
                } else {
                    this->keys_pressed--;
                    if (this->keys_pressed == 0) {
                        this->clear_report();
                    }
                }
            }
        }

        pinMode(ROW_PINS[row], INPUT_PULLUP);
    }
}

uint8_t debounce_input(uint8_t* debounce_state, int input)
{
    if (input) {
        if (*debounce_state > 0) {
            (*debounce_state)--;
        }
        if (*debounce_state == 0) {
            return DEBOUNCE_LOW;
        }
    } else {
        if (*debounce_state < DEBOUNCE_MAX) {
            (*debounce_state)++;
        }
        if (*debounce_state == DEBOUNCE_MAX) {
            return DEBOUNCE_HIGH;
        }
    }

    return DEBOUNCE_CHANGING;
}

void Keyboard::clear_report()
{
    memset(&(this->_keyReport), 0, sizeof(KeyReport));
}
