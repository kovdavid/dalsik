#pragma once

#include <stdint.h>
#include "array_utils.h"
#include "bit_utils.h"
#include "pin_utils.h"
#include "dalsik_hid_descriptor.h"
#include "matrix.h"
#include "dalsik_config.h"
#include "dalsik_eeprom.h"
#include "key_definitions.h"

typedef unsigned long millisec;

namespace Dalsik {
    void setup();
    void loop();
}

// Uncomment this to define KEYBOARD_SIDE in code instead of EEPROM
// See EEPROM::get_keyboard_side
// #define KEYBOARD_SIDE KEYBOARD_SIDE_LEFT
// #define KEYBOARD_SIDE KEYBOARD_SIDE_RIGHT
