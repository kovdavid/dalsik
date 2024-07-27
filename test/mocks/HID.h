#pragma once

#include <vector>
#include "dalsik_hid_descriptor.h"

class HIDSubDescriptor {
    public:
        HIDSubDescriptor(const void *d, const uint16_t l);
};

class TestHID {
    public:
        TestHID(void);
        int SendReport(uint8_t id, const void* data, int len);
        void AppendDescriptor(HIDSubDescriptor* node);
        void clear();

        std::vector<KeyboardHIDReport> keyboard_reports;
        std::vector<MouseHIDReport> mouse_reports;
};

TestHID& HID();
