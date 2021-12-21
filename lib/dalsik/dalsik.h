#pragma once

#include "dalsik_global.h"

#define DEBUG 0
#define DEBUG_KEYREPORT_BASE 0
#define DEBUG_KEYREPORT_SYSTEM 0
#define DEBUG_KEYREPORT_MULTIMEDIA 0

// Trigger key press after holding a key for X ms
#define DUAL_MODE_TIMEOUT_MS 1000
// Trigger the current tap_count if no tap occurs for this time
#define TAPDANCE_TIMEOUT_MS 300

// What to do, if other key is pressed before the dual key?
// 0 - send the dual key's modifier
// 1 - determine at the next key_press (modifier) or timeout (normal key) or release of the dual key (also normal key)
#define LAZY_DUAL_KEYS 1

namespace Dalsik {
    void setup();
    void loop();
}
