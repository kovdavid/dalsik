#pragma once

#define PIN_B(x) (0x30+x)
#define PIN_C(x) (0x60+x)
#define PIN_D(x) (0x90+x)
#define PIN_E(x) (0xC0+x)
#define PIN_F(x) (0xF0+x)

#define KEYBOARD_ROWS 4
#define KEYBOARD_COLS 6
#define ONE_SIDE_KEYS KEYBOARD_ROWS*KEYBOARD_COLS

const uint8_t ROW_PINS[KEYBOARD_ROWS] = {
    PIN_D(7), PIN_E(6), PIN_B(4), PIN_B(5)
};

const uint8_t COL_PINS[KEYBOARD_COLS] = {
    PIN_F(6), PIN_F(7), PIN_B(1), PIN_B(3), PIN_B(2), PIN_B(6)
};
