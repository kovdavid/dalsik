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

// Turn off the keyboard with this PIN
#define ON_OFF_PIN PIN_C(6)

#define LED_PIN PIN_D(4)

#define MAX_LAYER_COUNT 8
#define LAYER_HISTORY_CAPACITY 5
#define MAX_TAPDANCE_KEYS 16
#define MAX_TAPDANCE_TAPS 3

#define ROW_UNKNOWN 255
#define COL_UNKNOWN 255

#define KEYBOARD_SIDE_LEFT  0x00
#define KEYBOARD_SIDE_RIGHT 0x01

typedef struct {
    uint8_t type;
    uint8_t key;
} EEPROM_KeyInfo;

typedef struct {
    uint8_t type;
    uint8_t key;
    uint8_t row;
    uint8_t col;
} KeyInfo;

#define EVENT_NONE 0x00
#define EVENT_KEY_PRESS 0x01
#define EVENT_KEY_RELEASE 0x02

typedef struct {
    uint8_t type;
    uint8_t row;
    uint8_t col;
} ChangedKeyCoords;

#define DUAL_MODE_NOT_PRESSED     0x00
#define DUAL_MODE_PENDING         0x01
#define DUAL_MODE_PRESS_KEY       0x02
#define DUAL_MODE_HOLD_MODIFIER   0x03
#define DUAL_MODE_HOLD_LAYER      0x04

typedef struct {
    uint8_t mode;
    KeyInfo key_info;
    unsigned long last_press_ts;
} DualKeyState;

#define HOLD_OR_TOGGLE_NOT_PRESSED 0x00
#define HOLD_OR_TOGGLE_PENDING     0x01
#define HOLD_OR_TOGGLE_HOLD_LAYER  0x02

typedef struct {
    uint8_t mode;
    KeyInfo key_info;
} LayerHoldOrToggleState;

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
