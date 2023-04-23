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

TapDanceHandler::TapDanceHandler(Keyboard *next_event_handler, KeyMap *keymap) {
    this->next_event_handler = next_event_handler;
    this->keymap = keymap;
    this->state = {};
}

void TapDanceHandler::handle_key_event(KeyEvent event) {
    uint8_t action = CALL_NEXT_EVENT_HANDLER;

#if TAPDANCE_ENABLED
    if (!event.key_looked_up && event.type != EVENT_TIMEOUT) {
        // Most of the time `event.key_looked_up` will be false, so we will do
        // the lookup here. However, when a combo is triggered, the KeyInfo will
        // be set in the event.
        event.key_info = this->keymap->get_key(event.coords);
        event.key_looked_up = true;
    }

    if (event.type == EVENT_KEY_PRESS) {
        action = this->handle_key_press(event);
    } else if (event.type == EVENT_KEY_RELEASE) {
        action = this->handle_key_release(event);
    } else if (event.type == EVENT_TIMEOUT) {
        action = this->handle_timeout(event);
    }
#endif

    if (action == CALL_NEXT_EVENT_HANDLER) {
        this->next_event_handler->handle_key_event(event);
    }
}

uint8_t TapDanceHandler::handle_key_press(KeyEvent event) {
    if (this->state.pending_tapdance_start) {
        if (event.key_info.type == KEY_TAPDANCE) {
            if (event.key_info.key == this->state.key_event.key_info.key) {
                // We basically update event.type + event.timestamp
                this->state.key_event = event;
                this->state.tap_count++;

                TapDance *td = TAPDANCE_INDEX(this->state.key_event.key_info.key);
                if (this->state.tap_count == td->target_keys_size) {
                    this->trigger_tapdance(event.timestamp);
                }

                PRINT_INTERNAL_STATE

                return SKIP_NEXT_EVENT_HANDLER;
            } else {
                // Different TapDance key
                this->trigger_tapdance(event.timestamp);

                // Call "self" so that we start a new pending tapdance for the new key
                return this->handle_key_press(event);
            }
        } else {
            this->trigger_tapdance(event.timestamp);

            PRINT_INTERNAL_STATE

            return CALL_NEXT_EVENT_HANDLER;
        }
    } else {
        if (event.key_info.type == KEY_TAPDANCE) {
            this->state.pending_tapdance_start = event.timestamp;
            this->state.tap_count = 1;
            this->state.key_event = event;

            PRINT_INTERNAL_STATE

            return SKIP_NEXT_EVENT_HANDLER;
        } else {
            return CALL_NEXT_EVENT_HANDLER;
        }
    }
}

uint8_t TapDanceHandler::handle_key_release(KeyEvent event) {
    if (event.key_info.equals(this->state.key_event.key_info)) {
        // We basically update event.type + event.timestamp
        this->state.key_event = event;
    }
    // We always call next_event_handler. If we've already triggered a TapDance target key,
    // then we need to release it based on its KeyCoords. Otherwise these KeyCoords won'td
    // be in PressedKeys and so this event will be ignored.
    return CALL_NEXT_EVENT_HANDLER;
}

uint8_t TapDanceHandler::handle_timeout(KeyEvent event) {
    if (!this->state.pending_tapdance_start) {
        return CALL_NEXT_EVENT_HANDLER;
    }

    millisec threshold_timestamp =
        this->state.key_event.type == EVENT_KEY_PRESS
        ? TAPDANCE_HOLD_TRIGGER_THRESHOLD_MS  // The TapDance key is still pressed
        : TAPDANCE_IDLE_TRIGGER_THRESHOLD_MS; // The TapDance key released

    if (this->state.key_event.timestamp + threshold_timestamp < event.timestamp) {
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
    TapDance *td = TAPDANCE_INDEX(this->state.key_event.key_info.key);
    KeyInfo key_info = td->get_key_for_tap(this->state.tap_count, this->state.key_event.coords);

    KeyEvent event = KeyEvent(
        EVENT_KEY_PRESS,
        this->state.key_event.coords,
        key_info,
        now
    );

    this->next_event_handler->handle_key_event(event);

    if (this->state.key_event.type == EVENT_KEY_RELEASE) {
        // If the TapDance key is released, we immediately send the release event
        event.type = EVENT_KEY_RELEASE;
        this->next_event_handler->handle_key_event(event);
    }

    this->state.pending_tapdance_start = 0;
    this->state.tap_count = 0;
    this->state.key_event = {};
}

void TapDanceHandler::print_internal_state(millisec now) {
    Serial.print("\nTapDanceHandler pending_tapdance_start:");
    Serial.print(this->state.pending_tapdance_start);
    Serial.print(" now-pending_tapdance_start:");
    Serial.print(now - this->state.pending_tapdance_start);
    Serial.print(" tap_count:");
    Serial.print(this->state.tap_count);
    Serial.print(" key_event:");
    if (this->state.key_event.type == EVENT_KEY_PRESS) {
        Serial.print("P");
    } else if (this->state.key_event.type == EVENT_KEY_RELEASE) {
        Serial.print("R");
    }
    Serial.print("|R:");
    Serial.print(this->state.key_event.coords.row);
    Serial.print("-C:");
    Serial.print(this->state.key_event.coords.col);
    Serial.print("|key_looked_up:");
    Serial.print(this->state.key_event.key_looked_up);
    Serial.print("|KeyInfo(");
    Serial.print(this->state.key_event.key_info.type);
    Serial.print(",");
    Serial.print(this->state.key_event.key_info.layer);
    Serial.print(",");
    Serial.print(this->state.key_event.key_info.mod);
    Serial.print(",");
    Serial.print(this->state.key_event.key_info.key);
    Serial.print(")|timestamp:");
    Serial.print(this->state.key_event.timestamp);
    Serial.print(" now-timestamp:");
    Serial.print(now - this->state.key_event.timestamp);
    Serial.print("\n");
}
