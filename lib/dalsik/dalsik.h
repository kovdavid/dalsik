#pragma once

#include "dalsik_global.h"

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef DEBUG_KEYREPORT_BASE
#define DEBUG_KEYREPORT_BASE 0
#endif

#ifndef DEBUG_KEYREPORT_SYSTEM
#define DEBUG_KEYREPORT_SYSTEM 0
#endif

#ifndef DEBUG_KEYREPORT_MULTIMEDIA
#define DEBUG_KEYREPORT_MULTIMEDIA 0
#endif


// Trigger key press after holding a key for X ms
#define DUAL_MODE_TIMEOUT_MS 1000
// Trigger the current tap_count if no tap occurs for this time
#define TAPDANCE_TIMEOUT_MS 300
// Trigger timed dual key secondary action (modifier, layer press) after X ms
#define TIMED_DUAL_KEY_THRESHOLD_MS 300

namespace Dalsik {
    void setup();
    void loop();
}

// Uncomment this to define KEYBOARD_SIDE in code instead of EEPROM
// See EEPROM::get_keyboard_side
// #define KEYBOARD_SIDE KEYBOARD_SIDE_LEFT
// #define KEYBOARD_SIDE KEYBOARD_SIDE_RIGHT
