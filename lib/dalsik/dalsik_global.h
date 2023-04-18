#pragma once

#include <stdint.h>

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

// Turn off the keyboard with this PIN
#define ON_OFF_PIN PIN_C(6)

#ifdef LED_ENABLED
#define LED_PIN PIN_D(4)
#endif

#define MAX_LAYER_COUNT 8
#define LAYER_HISTORY_CAPACITY 5
#define MAX_TAPDANCE_KEYS 16
#define MAX_TAPDANCE_TAPS 3

#define KEYBOARD_SIDE_LEFT  0x00
#define KEYBOARD_SIDE_RIGHT 0x01

#define COMBO_ACTIVATION_TIMEOUT_MS 75
#define COMBO_START_THRESHOLD_MS 300

// If we tap then press&hold a DTH (Dual-tap-hold) key within the threshold,
// we automatically activate the key instead of the modifier.
// E.g. Tapping and holding a `D(SHIFT, SPACE)` key will send `SPACE` immediately on the second press
#define DUAL_TAP_HOLD_THRESHOLD_MS 300

#define COORD_UNKNOWN 255
#define COORD_COMBO 127
#define COORDS_INVALID KeyCoords { COORD_UNKNOWN, COORD_UNKNOWN }

class KeyCoords {
    public:
        uint8_t row;
        uint8_t col;

        bool equals(KeyCoords other) {
            return this->row == other.row && this->col == other.col;
        }
};

typedef unsigned long millisec;

#define EVENT_NONE 0x00
#define EVENT_KEY_PRESS 0x01
#define EVENT_KEY_RELEASE 0x02
#define EVENT_TIMEOUT 0x03

typedef struct {
    uint8_t type;
    KeyCoords coords;
} ChangedKeyEvent;

typedef struct {
    uint8_t key_reported;
    uint8_t key_pressed;
    uint8_t tap_count;
} TapDanceState;

const uint8_t ROW_PINS[ROW_PIN_COUNT] = {
    PIN_D(7), PIN_E(6), PIN_B(4), PIN_B(5)
};
const uint8_t COL_PINS[ONE_SIDE_COL_PIN_COUNT] = {
    PIN_F(6), PIN_F(7), PIN_B(1), PIN_B(3), PIN_B(2), PIN_B(6)
};

#define BASE_KEYBOARD_REPORT_ID       0x01
#define SYSTEM_KEYBOARD_REPORT_ID     0x02
#define MULTIMEDIA_KEYBOARD_REPORT_ID 0x03
#define MOUSE_REPORT_ID               0x04

#define BASE_HID_REPORT_KEYS 6

typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[BASE_HID_REPORT_KEYS];
} BaseHIDReport;

typedef struct {
    uint8_t key;
} SystemHIDReport;

typedef struct {
    uint8_t key;
    uint8_t prefix;
} MultimediaHIDReport;

typedef struct {
    uint8_t buttons;
    int8_t x;
    int8_t y;
    int8_t wheel;
} MouseHIDReport;

typedef struct {
    BaseHIDReport base;
    SystemHIDReport system;
    MultimediaHIDReport multimedia;
    MouseHIDReport mouse;
} HIDReports;
