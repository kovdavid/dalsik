#include "Arduino.h"
#include "HID.h"
#include "dalsik.h"
#include "keyboard_hid_desc.h"
#include "keymap.h"
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

    this->keys_pressed = 0;
    keyreport_clear(&(this->keyreport));
}

void Keyboard::loop(unsigned long now_msec)
{
    this->matrix_scan(now_msec);
}

void Keyboard::send_report() {
    HID().SendReport(KEYBOARD_REPORT_ID, &(this->keyreport), sizeof(KeyReport));
}

void Keyboard::matrix_scan(unsigned long now_msec)
{
    for (int row = 0; row < ROW_PIN_COUNT; row++) {
        pinMode(ROW_PINS[row], OUTPUT);
        digitalWrite(ROW_PINS[row], LOW);

        for (int col = 0; col < COL_PIN_COUNT; col++) {
            uint8_t input = digitalRead(COL_PINS[col]);
            uint8_t debounced_input = this->debounce_input(row, col, input);

            if (debounced_input == DEBOUNCE_CHANGING) {
                continue; // Wait, till the value stabilizes
            }

            if (debounced_input != this->keystate[row][col]) {
                this->keystate[row][col] = debounced_input;

                KeyInfo changed_key = this->keymap.get_key(row, col);

                Serial.print("Changed_key:");
                Serial.print(changed_key.normal.key, HEX);
                Serial.print("\n");

                if (debounced_input == DEBOUNCE_MAX) {
                    this->keys_pressed++;
                    Serial.println("Pressed");
                    keyreport_press(&(this->keyreport), changed_key.normal.key);
                } else {
                    this->keys_pressed--;
                    Serial.println("Released");
                    keyreport_release(&(this->keyreport), changed_key.normal.key);
                    if (this->keys_pressed == 0) {
                        this->clear_report();
                    }
                }

//if DEBUG
                Serial.print("KeyReport:");
                Serial.print(this->keyreport.modifiers, HEX);
                Serial.print(this->keyreport.reserved, HEX);
                Serial.print(this->keyreport.keys[0], HEX);
                Serial.print(this->keyreport.keys[1], HEX);
                Serial.print(this->keyreport.keys[2], HEX);
                Serial.print(this->keyreport.keys[3], HEX);
                Serial.print(this->keyreport.keys[4], HEX);
                Serial.print(this->keyreport.keys[5], HEX);
                Serial.print("\n");
//endif

                this->send_report();
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
        if (debounce_state > 0) {
            this->debounce[row][col]--;
        }
        if (debounce_state == 0) {
            return DEBOUNCE_LOW;
        }
    } else {
        if (debounce_state < DEBOUNCE_MAX) {
            this->debounce[row][col]++;
        }
        if (debounce_state == DEBOUNCE_MAX) {
            return DEBOUNCE_MAX;
        }
    }

    return DEBOUNCE_CHANGING;
}

void Keyboard::clear_report()
{
    memset(&(this->keyreport), 0, sizeof(KeyReport));
}
