#pragma once

#define PIN_B(x) (0x30+x)
#define PIN_C(x) (0x60+x)
#define PIN_D(x) (0x90+x)
#define PIN_E(x) (0xC0+x)
#define PIN_F(x) (0xF0+x)

#ifdef FERRIS_SWEEP

#define KEYBOARD_ROWS 4
#define KEYBOARD_COLS 5

const uint8_t DIRECT_PINS[KEYBOARD_ROWS][KEYBOARD_COLS] = {
    { PIN_E(6), PIN_F(7), PIN_F(6), PIN_F(5), PIN_F(4) },
    { PIN_B(1), PIN_B(3), PIN_B(2), PIN_B(6), PIN_D(3) },
    { PIN_D(1), PIN_D(0), PIN_D(4), PIN_C(6), PIN_D(7) },
    {     0x00,     0x00,     0x00, PIN_B(4), PIN_B(5) },
}

#define SERIAL_PIN PIN_D(2)

#else // Lets Split

#define KEYBOARD_ROWS 4
#define KEYBOARD_COLS 6

const uint8_t ROW_PINS[KEYBOARD_ROWS] = {
    PIN_D(7), PIN_E(6), PIN_B(4), PIN_B(5)
};

const uint8_t COL_PINS[KEYBOARD_COLS] = {
    PIN_F(6), PIN_F(7), PIN_B(1), PIN_B(3), PIN_B(2), PIN_B(6)
};

#define SERIAL_PIN PIN_D(0)

#endif
