#pragma once

#include "dalsik.h"
#include "pressed_keys.h"

#define CURSOR_MAX_REPEAT     255
#define CURSOR_MOVE_DELTA     8
#define CURSOR_REPEATS_TO_MAX 30
#define CURSOR_MAX_SPEED      10
#define CURSOR_MOVE_MAX       (INT16_MAX - 1)
#define CURSOR_DELAY          300
#define CURSOR_INTERVAL       150

typedef struct {
    uint8_t repeat;
    millisec last_repeat;
    uint8_t active_directions;
} MouseCursorState;

// Approximation function; The Pro Micro does not do HW division
static inline int16_t multiply_by_invert_sqrt2(int16_t input) {
    int32_t result = (int32_t)input * 23170;
    result >>= 15;

    if (result > INT16_MAX) {
        return INT16_MAX;
    }
    if (result < INT16_MIN) {
        return INT16_MIN;
    }
    return (int16_t)result;
}

static inline int16_t divide_by_mousekey_repeats_to_max(int16_t input) {
    // 32768 = 2**15
    int32_t result = (int32_t)input * (32768 / CURSOR_REPEATS_TO_MAX);
    result >>= 15;

    if (result > INT16_MAX) {
        return INT16_MAX;
    }
    if (result < INT16_MIN) {
        return INT16_MIN;
    }
    return (int16_t)result;
}

class Mouse {
    TEST_FRIEND_CLASS
    private:
        MouseHIDReport current_hid_report;
        MouseHIDReport last_hid_report;
        MouseCursorState cursor_state;

        int16_t cursor_move_unit();
        bool cursor_should_repeat(millisec now);

    public:
        Mouse();

        void press_button(PressedKey *pk);
        void release_button(PressedKey *pk);

        void handle_timeout(millisec now);
        void press_cursor(PressedKey *pk);
        void release_cursor(PressedKey *pk, millisec now);

        void send_hid_report();
};
