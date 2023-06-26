#pragma once

#ifdef LED_ENABLED
#define LED_PIN PIN_D(4)
#endif

#define MAX_LAYER_COUNT 8
#define ACTIVATED_LAYERS_CAPACITY 5

// Trigger key press after holding a key for X ms
#define DUAL_MODE_TIMEOUT_MS 1000
// Trigger the current tap_count if no tap occurs for this time
#define TAPDANCE_TIMEOUT_MS 300
// Trigger timed dual key secondary action (modifier, layer press) after X ms
#define TIMED_DUAL_KEY_THRESHOLD_MS 300
// Trigger OSM upon tapping the key with no longer than this value
#define ONE_SHOT_MODIFIER_TAP_TIMEOUT_MS 300

// Time limit for a pending combo processing.
#define COMBO_PROCESSING_LIMIT_MS 75
// To start a combo processing, this amount of time needs to pass without
// any events. This is to prevent accidental combo firing when typing fast.
#define COMBO_START_THRESHOLD_MS 300

// If a tapdance key is held for this amount of time, we will trigger the target key for the current tap count
#define TAPDANCE_HOLD_TRIGGER_THRESHOLD_MS 300
// If this amount of time ellapses after tapping a tapdance key (and no other event occurs), we will trigger the
// target key for the current tap count
#define TAPDANCE_IDLE_TRIGGER_THRESHOLD_MS 300

#define CAPS_WORD_TIMEOUT_MS 2000

// If we tap then press&hold a DTH (Dual-tap-hold) key within the threshold,
// we automatically activate the key instead of the modifier.
// E.g. Tapping and holding a `D(SHIFT, SPACE)` key will send `SPACE` immediately on the second press
#define DUAL_TAP_HOLD_THRESHOLD_MS 300
