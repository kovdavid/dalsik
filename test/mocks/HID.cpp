#include "HID.h"

HIDSubDescriptor::HIDSubDescriptor(
    const void *d UNUSED,
    const uint16_t l UNUSED
) {
}

TestHID& HID()
{
	static TestHID obj;
	return obj;
}

void TestHID::AppendDescriptor(HIDSubDescriptor *node UNUSED) {
}

int TestHID::SendReport(uint8_t type, const void* data, int len) {
    if (type == BASE_KEYBOARD_REPORT_ID) {
        BaseHIDReport r;
        memcpy(&r, data, len);
        this->base_hid_reports.push_back(r);
    } else if (type == SYSTEM_KEYBOARD_REPORT_ID) {
        SystemHIDReport r;
        memcpy(&r, data, len);
        this->system_hid_reports.push_back(r);
    } else if (type == MULTIMEDIA_KEYBOARD_REPORT_ID) {
        MultimediaHIDReport r;
        memcpy(&r, data, len);
        this->multimedia_hid_reports.push_back(r);
    }

    return 0;
}


TestHID::TestHID(void) {}
