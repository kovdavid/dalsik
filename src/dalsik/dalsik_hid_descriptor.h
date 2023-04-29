#pragma once

#include <avr/pgmspace.h>

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

const uint8_t KEYBOARD_HID_DESCRIPTOR[] PROGMEM = {
    //  Base Keyboard
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x06, // USAGE (Keyboard)
    0xA1, 0x01, // COLLECTION (Application)
    0x85, BASE_KEYBOARD_REPORT_ID,
    0x05, 0x07, //   USAGE_PAGE (Keyboard)

    0x19, 0xE0, //   USAGE_MINIMUM (Keyboard Left Control)
    0x29, 0xE7, //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00, //   LOGICAL_MINIMUM (0)
    0x25, 0x01, //   LOGICAL_MAXIMUM (1)
    0x95, 0x08, //   REPORT_COUNT (8)
    0x75, 0x01, //   REPORT_SIZE (1)
    0x81, 0x02, //   INPUT (Data,Var,Abs)

    0x95, 0x01, //   REPORT_COUNT (1)
    0x75, 0x08, //   REPORT_SIZE (8)
    0x81, 0x03, //   INPUT (Cnst,Var,Abs)

    0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0xFF, //   USAGE_MAXIMUM (Keyboard Application)
    0x15, 0x00, //   LOGICAL_MINIMUM (0)
    0x25, 0xFF, //   LOGICAL_MAXIMUM (255)
    0x95, 0x06, //   REPORT_COUNT (6)
    0x75, 0x08, //   REPORT_SIZE (8)
    0x81, 0x00, //   INPUT (Data,Ary,Abs)
    0xC0,       // END_COLLECTION - Base Keyboard

    // System Keyboard
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x80, // USAGE (System Control)
    0xA1, 0x01, // COLLECTION (Application)
    0x85, SYSTEM_KEYBOARD_REPORT_ID,
    0x19, 0x81, //   USAGE_MINIMUM (System Power Down)
    0x29, 0x83, //   USAGE_MAXIMUM (System Wake Up)
    0x15, 0x81, //   LOGICAL_MINIMUM (129)
    0x25, 0x83, //   LOGICAL_MAXIMUM (131)
    0x75, 0x08, //   REPORT_SIZE (8)
    0x95, 0x01, //   REPORT_COUNT (1)
    0x81, 0x00, //   INPUT (Data,Var,Abs)
    0xC0,       // END_COLLECTION

    // Multimedia Keyboard
    0x05, 0x0C,       // USAGE_PAGE (Consumer Devices)
    0x09, 0x01,       // USAGE (Consumer Control)
    0xA1, 0x01,       // COLLECTION (Application)
    0x85, MULTIMEDIA_KEYBOARD_REPORT_ID,
    0x19, 0x01,       //   USAGE_MINIMUM (+10)
    0x2A, 0x9C, 0x02, //   USAGE_MAXIMUM (AC Distribute Vertically)
    0x15, 0x01,       //   LOGICAL_MINIMUM (1)
    0x26, 0x9C, 0x02, //   LOGICAL_MAXIMUM (668)
    0x75, 0x10,       //   REPORT_SIZE (16)
    0x95, 0x01,       //   REPORT_COUNT (1)
    0x81, 0x00,       //   INPUT (Data,Var,Abs)
    0xC0,             // END_COLLECTION

    // Mouse
    0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,       // USAGE (Mouse)
    0xA1, 0x01,       // COLLECTION (Application)
    0x09, 0x01,       //   USAGE (Pointer)
    0xA1, 0x00,       //   COLLECTION (Physical)
    0x85, MOUSE_REPORT_ID,
    0x05, 0x09,       //     USAGE_PAGE (Button)
    0x19, 0x01,       //     USAGE_MINIMUM
    0x29, 0x05,       //     USAGE_MAXIMUM
    0x15, 0x00,       //     LOGICAL_MINIMUM (0)
    0x25, 0x01,       //     LOGICAL_MAXIMUM (1)
    0x95, 0x05,       //     REPORT_COUNT (5)
    0x75, 0x01,       //     REPORT_SIZE (1)
    0x81, 0x02,       //     INPUT (Data,Var,Abs)
    0x95, 0x01,       //     REPORT_COUNT (1)
    0x75, 0x03,       //     REPORT_SIZE (3)
    0x81, 0x03,       //     INPUT (Const,Var,Abs)
    0x05, 0x01,       //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,       //     USAGE (X)
    0x09, 0x31,       //     USAGE (Y)
    0x09, 0x38,       //     USAGE (Wheel)
    0x15, 0x81,       //     LOGICAL_MINIMUM (-127)
    0x25, 0x7F,       //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,       //     REPORT_SIZE (8)
    0x95, 0x03,       //     REPORT_COUNT (3)
    0x81, 0x06,       //     INPUT (Data,Var,Rel)
    0xC0,             //   END_COLLECTION
    0xC0,             // END COLLECTION
};
