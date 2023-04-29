#pragma once

#define PIN_B(x) (0x30+x)
#define PIN_C(x) (0x60+x)
#define PIN_D(x) (0x90+x)
#define PIN_E(x) (0xC0+x)
#define PIN_F(x) (0xF0+x)

#define ROW_PIN_COUNT 4
#define ONE_SIDE_COL_PIN_COUNT 6
#define ONE_SIDE_KEYS ROW_PIN_COUNT*ONE_SIDE_COL_PIN_COUNT
#define KEY_COUNT 2 * ONE_SIDE_KEYS

#define KEYBOARD_ROWS ROW_PIN_COUNT
#define KEYBOARD_COLS 2 * ONE_SIDE_COL_PIN_COUNT

const uint8_t ROW_PINS[ROW_PIN_COUNT] = {
    PIN_D(7), PIN_E(6), PIN_B(4), PIN_B(5)
};

const uint8_t COL_PINS[ONE_SIDE_COL_PIN_COUNT] = {
    PIN_F(6), PIN_F(7), PIN_B(1), PIN_B(3), PIN_B(2), PIN_B(6)
};
