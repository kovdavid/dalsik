#pragma once

#include <vector>
#include "dalsik.h"

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

        std::vector<BaseHIDReport> base_hid_reports;
        std::vector<SystemHIDReport> system_hid_reports;
        std::vector<MultimediaHIDReport> multimedia_hid_reports;
};

TestHID& HID();
