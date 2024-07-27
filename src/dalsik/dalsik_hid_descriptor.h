#pragma once

#include <avr/pgmspace.h>

enum hid_report_ids {
    KEYBOARD_REPORT_ID = 0x01,
    DESKTOP_REPORT_ID,
    CONSUMER_REPORT_ID,
    MOUSE_REPORT_ID
};

#define KEYBOARD_REPORT_KEYS 6

typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[KEYBOARD_REPORT_KEYS];
} KeyboardHIDReport;

typedef struct {
    uint8_t key;
} DesktopHIDReport;

typedef struct {
    uint16_t key;
} ConsumerHIDReport;

typedef struct {
    uint8_t buttons;
    int16_t x;
    int16_t y;
    int8_t wheel;
} __attribute__((packed)) MouseHIDReport;

typedef struct {
    KeyboardHIDReport keyboard;
    DesktopHIDReport desktop;
    ConsumerHIDReport consumer;
} HIDReports;

const uint8_t KEYBOARD_HID_DESCRIPTOR[] PROGMEM = {
    //  Base Keyboard
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x06, // USAGE (Keyboard)
    0xA1, 0x01, // COLLECTION (Application)
    0x85, KEYBOARD_REPORT_ID,
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
    0x85, DESKTOP_REPORT_ID,
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
    0x85, CONSUMER_REPORT_ID,
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
    0x85, MOUSE_REPORT_ID,
    0x09, 0x01,       //   USAGE (Pointer)
    0xA1, 0x00,       //   COLLECTION (Physical)

    // Button (8 bits)
    0x05, 0x09,       //     USAGE_PAGE (Button)
    0x19, 0x01,       //     USAGE_MINIMUM
    0x29, 0x08,       //     USAGE_MAXIMUM
    0x15, 0x00,       //     LOGICAL_MINIMUM (0)
    0x25, 0x01,       //     LOGICAL_MAXIMUM (1)
    0x95, 0x08,       //     REPORT_COUNT (8)
    0x75, 0x01,       //     REPORT_SIZE (1)
    0x81, 0x02,       //     INPUT (Data,Var,Abs)
    // X/Y position (4 bytes)
    0x05, 0x01,       //     Usage Page (Generic Desktop)
    0x09, 0x30,       //     Usage (X)
    0x09, 0x31,       //     Usage (Y)
    0x16, 0x01, 0x80, // Logical Minimum (-32767)
    0x26, 0xFF, 0x7F, // Logical Maximum (32767)
    0x95, 0x02,       // Report Count (2)
    0x75, 0x10,       // Report Size (16)
    0x81, 0x06,       //     Input (Data, Variable, Relative)
    // Vertical wheel (1 byte)
    0x09, 0x38,       //     Usage (Wheel)
    0x15, 0x81,       //     Logical Minimum (-127)
    0x25, 0x7F,       //     Logical Maximum (127)
    0x95, 0x01,       //     Report Count (1)
    0x75, 0x08,       //     Report Size (8)
    0x81, 0x06,       //     Input (Data, Variable, Relative)
    // END
    0xC0,             //   END_COLLECTION
    0xC0,             // END COLLECTION
};
