#ifndef DALSIK_H
#define DALSIK_H

#define DEBUG 0
#define DEBUG_KEYREPORT 0

#define IS_MASTER 1

#define MASTER_SIDE_LEFT 0x00
#define MASTER_SIDE_RIGHT 0x01
// Which side has the USB cable plugged?
#define MASTER_SIDE MASTER_SIDE_LEFT

// Use I2C or serial?
#define USE_I2C 0
#define I2C_MASTER_ADDRESS 0x01
#define I2C_SLAVE_ADDRESS  0x02

// Turn off the keyboard with this PIN
#define ON_OFF_PIN 4

// Trigger key press after holding a key for X ms
#define DUAL_MODE_TIMEOUT_MS 1000
// Trigger the current tap_count if no tap occurs for this time
#define TAPDANCE_TIMEOUT_MS 300

#define ROW_PIN_COUNT 4
#define ONE_SIDE_COL_PIN_COUNT 6
#define BOTH_SIDE_COL_PIN_COUNT 2*ONE_SIDE_COL_PIN_COUNT
#define KEY_COUNT ROW_PIN_COUNT * BOTH_SIDE_COL_PIN_COUNT

#define PIN_B(x) (0x30+x)
#define PIN_C(x) (0x60+x)
#define PIN_D(x) (0x90+x)
#define PIN_E(x) (0xC0+x)
#define PIN_F(x) (0xF0+x)

const uint8_t ROW_PINS[ROW_PIN_COUNT] = {
    PIN_D(7), PIN_E(6), PIN_B(4), PIN_B(5)
};
const uint8_t COL_PINS[ONE_SIDE_COL_PIN_COUNT] = {
    PIN_F(6), PIN_F(7), PIN_B(1), PIN_B(3), PIN_B(2), PIN_B(6)
};

#endif
