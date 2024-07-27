#include "Arduino.h"
#include "HID.h"

#include "mouse.h"

Mouse::Mouse()
    : current_hid_report()
    , last_hid_report()
    , cursor_state()
{}

void Mouse::press_button(PressedKey *pk) {
    BIT_SET(this->current_hid_report.buttons, pk->key.code);
}

void Mouse::release_button(PressedKey *pk) {
    BIT_CLEAR(this->current_hid_report.buttons, pk->key.code);
}

void Mouse::press_cursor(PressedKey *pk) {
    int16_t move_unit = this->cursor_move_unit();
    uint8_t direction = pk->key.code;

    BIT_SET(this->cursor_state.active_directions, direction);
    this->cursor_state.last_repeat = pk->timestamp;

    int16_t* x = &(this->current_hid_report.x);
    int16_t* y = &(this->current_hid_report.y);

    if (direction & MOUSE_CURSOR_UP) {
        *y = -move_unit;
    }
    if (direction & MOUSE_CURSOR_DOWN) {
        *y = move_unit;
    }
    if (direction & MOUSE_CURSOR_LEFT) {
        *x = -move_unit;
    }
    if (direction & MOUSE_CURSOR_RIGHT) {
        *x = move_unit;
    }
}

void Mouse::release_cursor(PressedKey *pk, millisec now) {
    uint8_t direction = pk->key.code;

    BIT_CLEAR(this->cursor_state.active_directions, direction);
    if (this->cursor_state.active_directions) {
        this->cursor_state.last_repeat = now;
    } else {
        this->cursor_state.repeat = 0;
    }

    int16_t* x = &(this->current_hid_report.x);
    int16_t* y = &(this->current_hid_report.y);

    if (direction & MOUSE_CURSOR_UP && *y < 0) {
        *y = 0;
    }
    if (direction & MOUSE_CURSOR_DOWN && *y > 0) {
        *y = 0;
    }
    if (direction & MOUSE_CURSOR_LEFT && *x < 0) {
        *x = 0;
    }
    if (direction & MOUSE_CURSOR_RIGHT && *x > 0) {
        *x = 0;
    }
}

int16_t Mouse::cursor_move_unit() {
    int32_t unit = 0;

    if (this->cursor_state.repeat == 0) {
        unit = CURSOR_MOVE_DELTA;
    } else if (this->cursor_state.repeat >= CURSOR_REPEATS_TO_MAX) {
        unit = CURSOR_MOVE_DELTA * CURSOR_MAX_SPEED;
    } else {
        unit = divide_by_mousekey_repeats_to_max(CURSOR_MOVE_DELTA * CURSOR_MAX_SPEED * this->cursor_state.repeat);
    }

    if (unit > CURSOR_MOVE_MAX) {
        return CURSOR_MOVE_MAX;
    }
    if (unit == 0) {
        return 1;
    }
    if (unit > INT16_MAX) {
        return INT16_MAX;
    }
    if (unit < INT16_MIN) {
        return INT16_MIN;
    }

    return (int16_t)unit;
}

bool Mouse::cursor_should_repeat(millisec now) {
    if (!this->cursor_state.active_directions) {
        return false;
    }

    millisec elapsed_time = now - this->cursor_state.last_repeat;

    if (this->cursor_state.repeat > 0) {
        return elapsed_time > CURSOR_INTERVAL;
    } else {
        return elapsed_time > CURSOR_DELAY;
    }
}

void Mouse::handle_timeout(millisec now) {
    if (!this->cursor_should_repeat(now)) {
        return;
    }

    if (this->cursor_state.repeat < CURSOR_MAX_REPEAT) {
        this->cursor_state.repeat++;
    }

    int16_t* x = &(this->current_hid_report.x);
    int16_t* y = &(this->current_hid_report.y);

    int16_t move_unit = this->cursor_move_unit();
    uint8_t directions = this->cursor_state.active_directions;

    if (directions & MOUSE_CURSOR_UP) {
        *y = -move_unit;
    }
    if (directions & MOUSE_CURSOR_DOWN) {
        *y = move_unit;
    }
    if (directions & MOUSE_CURSOR_LEFT) {
        *x = -move_unit;
    }
    if (directions & MOUSE_CURSOR_RIGHT) {
        *x = move_unit;
    }

    // diagonal move
    if (*x != 0 && *y != 0) {
        *x = multiply_by_invert_sqrt2(*x) || 1;
        *y = multiply_by_invert_sqrt2(*y) || 1;
    }

    this->cursor_state.last_repeat = now;
}


void Mouse::send_hid_report() {
    size_t report_size = sizeof(MouseHIDReport);

    MouseHIDReport* current = &(this->current_hid_report);
    MouseHIDReport* last = &(this->last_hid_report);

    if (memcmp(current, last, report_size) == 0) {
        return;
    }

#if DEBUG_KEYREPORT_MOUSE
    Serial.print(F("Mouse:"));
    Serial.print(current->buttons, HEX);
    Serial.print(F("|"));
    Serial.print((uint16_t)current->x, HEX);
    Serial.print(F("|"));
    Serial.print((uint16_t)current->y, HEX);
    Serial.print(F("|"));
    Serial.print((uint8_t)current->wheel, HEX);
    Serial.print(F("\n"));
#endif

    HID().SendReport(MOUSE_REPORT_ID, current, report_size);
    *last = *current;

    current->x = 0;
    current->y = 0;
}
