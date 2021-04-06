#ifndef DALSIK_H
#define DALSIK_H

#include "pin_utils.h"

#define DEBUG 0
#define DEBUG_KEYREPORT_BASE 0
#define DEBUG_KEYREPORT_SYSTEM 0
#define DEBUG_KEYREPORT_MULTIMEDIA 0

// Turn off the keyboard with this PIN
#define ON_OFF_PIN PIN_C(6)

#define LED_PIN PIN_D(4)

// Trigger key press after holding a key for X ms
#define DUAL_MODE_TIMEOUT_MS 1000
// Trigger the current tap_count if no tap occurs for this time
#define TAPDANCE_TIMEOUT_MS 300

#define ROW_PIN_COUNT 4
#define ONE_SIDE_COL_PIN_COUNT 6

// What to do, if other key is pressed before the dual key?
// 0 - send the dual key's modifier
// 1 - determine at the next key_press (modifier) or timeout (normal key) or release of the dual key (also normal key)
#define LAZY_DUAL_KEYS 1

const uint8_t ROW_PINS[ROW_PIN_COUNT] = {
    PIN_D(7), PIN_E(6), PIN_B(4), PIN_B(5)
};
const uint8_t COL_PINS[ONE_SIDE_COL_PIN_COUNT] = {
    PIN_F(6), PIN_F(7), PIN_B(1), PIN_B(3), PIN_B(2), PIN_B(6)
};

#endif
