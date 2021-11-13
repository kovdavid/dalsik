#pragma once

#include "dalsik.h"
#include "HID.h"

const uint8_t KEYBOARD_HID_DESC[] PROGMEM = {
    //  Base Keyboard
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x06, // USAGE (Keyboard)
    0xa1, 0x01, // COLLECTION (Application)
    0x85, BASE_KEYBOARD_REPORT_ID, // REPORT_ID
    0x05, 0x07, //   USAGE_PAGE (Keyboard)

    0x19, 0xe0, //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7, //   USAGE_MAXIMUM (Keyboard Right GUI)
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
    0xc0,       // END_COLLECTION - Base Keyboard

    // System Keyboard
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x80, // USAGE (System Control)
    0xa1, 0x01, // COLLECTION (Application)
    0x85, SYSTEM_KEYBOARD_REPORT_ID, // REPORT_ID
    0x19, 0x81, //   USAGE_MINIMUM (System Power Down)
    0x29, 0x83, //   USAGE_MAXIMUM (System Wake Up)
    0x15, 0x81, //   LOGICAL_MINIMUM (129)
    0x25, 0x83, //   LOGICAL_MAXIMUM (131)
    0x75, 0x08, //   REPORT_SIZE (8)
    0x95, 0x01, //   REPORT_COUNT (1)
    0x81, 0x00, //   INPUT (Data,Var,Abs)
    0xc0,       // END_COLLECTION

    // Multimedia Keyboard
    0x05, 0x0c,       // USAGE_PAGE (Consumer Devices)
    0x09, 0x01,       // USAGE (Consumer Control)
    0xa1, 0x01,       // COLLECTION (Application)
    0x85, MULTIMEDIA_KEYBOARD_REPORT_ID, // REPORT_ID
    0x19, 0x01,       //   USAGE_MINIMUM (+10)
    0x2a, 0x9C, 0x02, //   USAGE_MAXIMUM (AC Distribute Vertically)
    0x15, 0x01,       //   LOGICAL_MINIMUM (1)
    0x26, 0x9C, 0x02, //   LOGICAL_MAXIMUM (668)
    0x75, 0x10,       //   REPORT_SIZE (16)
    0x95, 0x01,       //   REPORT_COUNT (1)
    0x81, 0x00,       //   INPUT (Data,Var,Abs)
    0xc0              // END_COLLECTION
};

namespace DalsikHid {
    inline void init_descriptor() {
        static HIDSubDescriptor node(KEYBOARD_HID_DESC, sizeof(KEYBOARD_HID_DESC));
        HID().AppendDescriptor(&node);
    }

    inline void send_report(uint8_t id, const void* data, int len) {
        HID().SendReport(id, data, len);
    }
}
