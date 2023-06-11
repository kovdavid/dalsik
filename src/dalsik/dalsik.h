#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <avr/pgmspace.h>
#include <avr/io.h>

#include "array_utils.h"
#include "bit_utils.h"
#include "pin_utils.h"
#include "dalsik_hid_descriptor.h"
#include "matrix.h"
#include "dalsik_config.h"
#include "key_definitions.h"

typedef unsigned long millisec;

namespace Dalsik {
    void setup();
    void loop();
}

#if defined(TEST)
#define TEST_FRIEND_CLASS friend class TestFriendClass;
#define TAPDANCE_ENABLED 1
#define COMBOS_ENABLED 1
#define CAPS_WORD_TIMEOUT 1000
#else
#define TEST_FRIEND_CLASS
#endif

// Uncomment this to define KEYBOARD_SIDE in code instead of EEPROM
// See EEPROM::get_keyboard_side
// #define KEYBOARD_SIDE KEYBOARD_SIDE_LEFT
// #define KEYBOARD_SIDE KEYBOARD_SIDE_RIGHT
