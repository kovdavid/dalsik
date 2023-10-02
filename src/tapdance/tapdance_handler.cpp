#include "tapdance.h"
#include "tapdance_handler.h"
#include "keymap.h"
#include "keyboard.h"
#include "key_definitions.h"
#include "tapdance.h"

#define SKIP_NEXT_EVENT_HANDLER 0x01
#define CALL_NEXT_EVENT_HANDLER 0x02

#if DEBUG_TAPDANCE_HANDLER_STATE
#define PRINT_INTERNAL_STATE this->print_internal_state(event.timestamp);
#else
#define PRINT_INTERNAL_STATE
#endif

TapDanceHandler::TapDanceHandler(Keyboard *next_event_handler, KeyMap *keymap)
    : next_event_handler(next_event_handler)
    , keymap(keymap)
    , state({})
{}

void TapDanceHandler::handle_key_event(ExtendedKeyEvent event) {
    uint8_t action = CALL_NEXT_EVENT_HANDLER;

    if (tapdances_count > 0) {
        if (event.type == EVENT_KEY_PRESS) {
            this->next_event_handler->run_press_hooks();
        }

        event.look_up_key(this->keymap);

        if (event.type == EVENT_KEY_PRESS) {
            action = this->handle_key_press(event);
        } else if (event.type == EVENT_KEY_RELEASE) {
            action = this->handle_key_release(event);
        } else if (event.type == EVENT_TIMEOUT) {
            action = this->handle_timeout(event);
        }
    }

    if (action == CALL_NEXT_EVENT_HANDLER) {
        this->next_event_handler->handle_key_event(event);
    }
}

uint8_t TapDanceHandler::handle_key_press(ExtendedKeyEvent event) {
    if (this->state.pending_tapdance_start) {
        return this->resume_pending_tapdance(event);
    } else {
        if (event.key.type == KEY_TAPDANCE) {
            if (event.key.code < tapdances_count) {
                this->state.pending_tapdance_start = event.timestamp;
                this->state.pending_tap_count = 1;
                this->state.last_key_event = event;
            }

            PRINT_INTERNAL_STATE

            return SKIP_NEXT_EVENT_HANDLER;
        } else {
            return CALL_NEXT_EVENT_HANDLER;
        }
    }
}

uint8_t TapDanceHandler::resume_pending_tapdance(ExtendedKeyEvent event) {
    if (event.key.equals(this->state.last_key_event.key)) {
        this->state.last_key_event = event;
        this->state.pending_tap_count++;

        uint8_t tapdance_index = this->state.last_key_event.key.code;

        TapDance tapdance = tapdances[tapdance_index];
        if (this->state.pending_tap_count == tapdance.target_keys_size) {
            this->trigger_tapdance(event.timestamp);
        }

        PRINT_INTERNAL_STATE

        return SKIP_NEXT_EVENT_HANDLER;
    } else {
        this->trigger_tapdance(event.timestamp);

        PRINT_INTERNAL_STATE

        return CALL_NEXT_EVENT_HANDLER;
    }
}

uint8_t TapDanceHandler::handle_key_release(ExtendedKeyEvent event) {
    if (event.key.equals(this->state.last_key_event.key)) {
        // We basically update event.type + event.timestamp
        this->state.last_key_event = event;
    }
    // We always call next_event_handler. If we've already triggered a TapDance target key,
    // then we need to release it based on its KeyCoords. Otherwise these KeyCoords won't
    // be in PressedKeys and so this event will be ignored.
    return CALL_NEXT_EVENT_HANDLER;
}

uint8_t TapDanceHandler::handle_timeout(ExtendedKeyEvent event) {
    if (!this->state.pending_tapdance_start) {
        return CALL_NEXT_EVENT_HANDLER;
    }

    millisec threshold_timestamp =
        this->state.last_key_event.type == EVENT_KEY_PRESS
        ? TAPDANCE_HOLD_TRIGGER_THRESHOLD_MS  // The TapDance key is still pressed
        : TAPDANCE_IDLE_TRIGGER_THRESHOLD_MS; // The TapDance key is released

    if (this->state.last_key_event.timestamp + threshold_timestamp < event.timestamp) {
        this->trigger_tapdance(event.timestamp);

        PRINT_INTERNAL_STATE
    }

    return CALL_NEXT_EVENT_HANDLER;
}

// TapDance key is triggered either by:
// - handle_key_press
//   - when we reach the max tap count for our TapDance key
// - handle_timeout
//   - when the tapdance key is held for a certain amount of time
//   - when a certain amount of time passes since releasing the tapdance key
void TapDanceHandler::trigger_tapdance(millisec now) {
    uint8_t tapdance_index = this->state.last_key_event.key.code;
    TapDance tapdance = tapdances[tapdance_index];

    Key key = tapdance.get_key_for_tap(this->state.pending_tap_count, this->state.last_key_event.coords);

    ExtendedKeyEvent event = ExtendedKeyEvent(
        EVENT_KEY_PRESS,
        this->state.last_key_event.coords,
        key,
        now
    );

    this->next_event_handler->handle_key_event(event);

    if (this->state.last_key_event.type == EVENT_KEY_RELEASE) {
        // If the TapDance key is released, we immediately send the release event
        event.type = EVENT_KEY_RELEASE;
        this->next_event_handler->handle_key_event(event);
    }

    this->state = {};
}

void TapDanceHandler::print_internal_state(millisec now) {
    Serial.print("\nTapDanceHandler pending_tapdance_start:");
    Serial.print(this->state.pending_tapdance_start);
    Serial.print(" now-pending_tapdance_start:");
    Serial.print(now - this->state.pending_tapdance_start);
    Serial.print(" pending_tap_count:");
    Serial.print(this->state.pending_tap_count);
    Serial.print(" last_key_event:");
    if (this->state.last_key_event.type == EVENT_KEY_PRESS) {
        Serial.print("P");
    } else if (this->state.last_key_event.type == EVENT_KEY_RELEASE) {
        Serial.print("R");
    }
    Serial.print("|R:");
    Serial.print(this->state.last_key_event.coords.row);
    Serial.print("-C:");
    Serial.print(this->state.last_key_event.coords.col);
    Serial.print("|key_looked_up:");
    Serial.print(this->state.last_key_event.key_looked_up);
    Serial.print("|Key(");
    Serial.print(this->state.last_key_event.key.type);
    Serial.print(",");
    Serial.print(this->state.last_key_event.key.layer);
    Serial.print(",");
    Serial.print(this->state.last_key_event.key.modifiers);
    Serial.print(",");
    Serial.print(this->state.last_key_event.key.code);
    Serial.print(")|timestamp:");
    Serial.print(this->state.last_key_event.timestamp);
    Serial.print(" now-timestamp:");
    Serial.print(now - this->state.last_key_event.timestamp);
    Serial.print("\n");
}
