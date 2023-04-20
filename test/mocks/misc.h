#pragma once

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PROGMEM
#define HEX 16
#define BIN 2

class __FlashStringHelper;

#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(string_literal))
#define UNUSED __attribute__((unused))

// Base REPort COMPare
#define BREP_COMP(i, ...) TEST_CHECK(compare_base_report(i, __VA_ARGS__))

#define HID_SIZE_CHECK(expected) \
    TEST_CHECK(HID().base_hid_reports.size() == expected); \
    TEST_MSG("size: %lu expected:%d", HID().base_hid_reports.size(), expected)
