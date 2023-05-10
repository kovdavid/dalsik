#include "HID.h"

HIDSubDescriptor::HIDSubDescriptor(const void *d UNUSED, const uint16_t l UNUSED) {
}

TestHID& HID() {
    static TestHID obj;
    return obj;
}

void TestHID::AppendDescriptor(HIDSubDescriptor *node UNUSED) {
}

void TestHID::clear() {
    this->base_hid_reports.clear();
    this->system_hid_reports.clear();
    this->multimedia_hid_reports.clear();
}

int TestHID::SendReport(uint8_t type, const void* data, int len) {
    if (type == BASE_KEYBOARD_REPORT_ID) {
        this->base_hid_reports.push_back(*(BaseHIDReport*)data);
    } else if (type == SYSTEM_KEYBOARD_REPORT_ID) {
        this->system_hid_reports.push_back(*(SystemHIDReport*)data);
    } else if (type == MULTIMEDIA_KEYBOARD_REPORT_ID) {
        this->multimedia_hid_reports.push_back(*(MultimediaHIDReport*)data);
    }

    return 0;
}

TestHID::TestHID(void) {}
