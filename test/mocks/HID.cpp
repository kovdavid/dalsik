#include "HID.h"
#include "Arduino.h"

HIDSubDescriptor::HIDSubDescriptor(const void *d UNUSED, const uint16_t l UNUSED) {
}

TestHID& HID() {
    static TestHID obj;
    return obj;
}

void TestHID::AppendDescriptor(HIDSubDescriptor *node UNUSED) {
}

void TestHID::clear() {
    this->keyboard_reports.clear();
}

int TestHID::SendReport(uint8_t type, const void* data, int len) {
    if (type == KEYBOARD_REPORT_ID) {
        this->keyboard_reports.push_back(*(KeyboardHIDReport*)data);
    } else if (type == MOUSE_REPORT_ID) {
        this->mouse_reports.push_back(*(MouseHIDReport*)data);
    }

    return 0;
}

TestHID::TestHID(void) {}
