#include "Arduino.h"
#include "combos.h"
#include "combos_handler.h"
#include "combo_state.h"
#include "key_event.h"
#include "tapdance_handler.h"

#define SKIP_NEXT_EVENT_HANDLER 0x01
#define CALL_NEXT_EVENT_HANDLER 0x02

#if DEBUG_COMBOS_HANDLER_STATE
#define PRINT_INTERNAL_STATE this->print_internal_state(event.timestamp);
#else
#define PRINT_INTERNAL_STATE
#endif

CombosHandler::CombosHandler(TapDanceHandler* next_event_handler) {
    this->next_event_handler = next_event_handler;
    this->key_buffer = CombosKeyBuffer {};
    this->pending_combos_start = 0;
    this->last_passthrough_event = 0;
}

uint8_t CombosHandler::handle_key_event(KeyEvent event) {
    uint8_t action = CALL_NEXT_EVENT_HANDLER;

#ifdef COMBOS_ENABLED
    if (this->pending_combos_start) {
        action = this->resume_pending_combo_processing(event);
    } else {
        action = this->start_pending_combo_processing(event);
    }
#endif

    if (action == CALL_NEXT_EVENT_HANDLER) {
        this->next_event_handler->handle_key_event(event);
    }

    return action;
}

// If the event key is part of a combo, then start a pending combo processing
uint8_t CombosHandler::start_pending_combo_processing(KeyEvent event) {
    if (event.type == EVENT_TIMEOUT) {
        // No combo is active/pending; just pass the execution to next_event_handler
        return CALL_NEXT_EVENT_HANDLER;
    }

    if (event.type == EVENT_KEY_RELEASE) {
        uint8_t action = CALL_NEXT_EVENT_HANDLER;

        CombosBufferedKey* cbk = this->key_buffer.find(event.coords);
        if (cbk == NULL) return action;

        if (cbk->part_of_active_combo) {
            this->release_active_combo_key(cbk, event.timestamp);
            action = SKIP_NEXT_EVENT_HANDLER;
        }

        cbk->clear();
        this->key_buffer.normalize();

        if (action == SKIP_NEXT_EVENT_HANDLER) {
            PRINT_INTERNAL_STATE
        }

        return action;
    }

    // We need to have COMBO_START_THRESHOLD_MS milliseconds without any events
    // to start combo processing. This is to prevent accidental combo firing
    // when typing
    if (this->last_passthrough_event + COMBO_START_THRESHOLD_MS > event.timestamp) {
        this->last_passthrough_event = event.timestamp;
        return CALL_NEXT_EVENT_HANDLER;
    }

    // EVENT_KEY_PRESS
    uint8_t affected_combos_count = 0;
    uint8_t disabled_combos_count = 0;
    for (uint8_t i = 0; i < COMBOS_COUNT; i++) {
        ComboState* combo_state = COMBO_STATE(i);
        if (combo_state->is_activated()) continue;

        if (combo_state->update_state_if_affected(event.coords, event.timestamp)) {
            affected_combos_count++;
        } else {
            disabled_combos_count++;
            combo_state->set_disabled();
        }
    }

    if (affected_combos_count > 0) {
        // The event was for a combo key, so we keep it
        this->key_buffer.add(event.coords, event.timestamp);
        this->pending_combos_start = event.timestamp;
        PRINT_INTERNAL_STATE
        return SKIP_NEXT_EVENT_HANDLER;
    } else {
        // Clear DISABLED combos marked above
        if (disabled_combos_count) {
            for (uint8_t i = 0; i < COMBOS_COUNT; i++) {
                ComboState* combo_state = COMBO_STATE(i);
                if (combo_state->is_activated()) continue;

                combo_state->clear_disabled();
            }

            PRINT_INTERNAL_STATE
        }

        this->last_passthrough_event = event.timestamp;

        // No combo is active/pending; just pass the execution to next_event_handler
        return CALL_NEXT_EVENT_HANDLER;
    }
}

uint8_t CombosHandler::resume_pending_combo_processing(KeyEvent event) {
    if (event.type == EVENT_KEY_PRESS) {
        return this->resume_pending_combo_processing_press(event);
    } else if (event.type == EVENT_KEY_RELEASE) {
        return this->resume_pending_combo_processing_release(event);
    } else {
        return this->resume_pending_combo_processing_timeout(event);
    }
}

uint8_t CombosHandler::resume_pending_combo_processing_press(KeyEvent event) {
    uint8_t affected_combos_count = 0;
    uint8_t disabled_combos_count = 0;
    int8_t fully_pressed_combo_index = -1;

    // Mark/update combos that contain the newly pressed key
    for (uint8_t i = 0; i < COMBOS_COUNT; i++) {
        ComboState* combo_state = COMBO_STATE(i);
        if (combo_state->is_activated()) continue;
        if (combo_state->is_disabled()) continue;

        if (combo_state->update_state_if_affected(event.coords, event.timestamp)) {
            affected_combos_count++;

            if (combo_state->is_fully_pressed()) {
                fully_pressed_combo_index = i;
            }
        } else {
            // We cannot clear them yet, because if the newly pressed key
            // is not part of any pending combos, then we need to check
            // if there was a pending fully pressed combo before this key
            // and we need to activate that combo.
            disabled_combos_count++;
            combo_state->set_disabled();
        }
    }


    // The newly pressed key is part of a pending combo
    if (affected_combos_count > 0) {
        this->key_buffer.add(event.coords, event.timestamp);

        // Cleanup any newly disabled (former) pending combos from the loop above
        if (disabled_combos_count > 0) {
            for (uint8_t i = 0; i < COMBOS_COUNT; i++) {
                ComboState* combo_state = COMBO_STATE(i);
                if (combo_state->is_activated()) continue;

                if (combo_state->is_disabled()) {
                    combo_state->clear_state_keep_flags();
                }
            }
        }

        // If only 1 pending combo remains (that is also fully pressed),
        // we can go ahead and activate the combo. There can't be any more
        // keys pressed that would be part of the pending combos.
        if (affected_combos_count == 1 && fully_pressed_combo_index >= 0) {
            // Only one combo is pending and it is fully pressed
            this->activate_combo(fully_pressed_combo_index);
        } else {
            // There are either more than 1 affected pending combos, or the
            // only pending combo is not fully pressed yet.
            // Do nothing at this point.
        }

        PRINT_INTERNAL_STATE

        return SKIP_NEXT_EVENT_HANDLER;
    } else if (disabled_combos_count > 0) {
        // So there were no affected pending combos, but we've disabled
        // some previously pending combos.
        //

        int8_t fully_pressed_combo_index = -1;

        for (uint8_t i = 0; i < COMBOS_COUNT; i++) {
            ComboState* combo_state = COMBO_STATE(i);
            if (combo_state->is_activated()) continue;

            if (combo_state->is_fully_pressed()) {
                fully_pressed_combo_index = i;
            }
        }

        // We must check if any of those were fully_pressed, in which case
        // we will activate that combo. We will also recursively call
        // this->handle_key_event, because the newly pressed key could start
        // a new pending combo processing.
        //
        // Otherwise we can abort the whole pending combos processing, as we
        // don't have any fully pressed pending combo.
        if (fully_pressed_combo_index >= 0) {
            this->activate_combo(fully_pressed_combo_index);
            PRINT_INTERNAL_STATE
            return this->handle_key_event(event);
        } else {
            this->abort_pending_combos_processing();
            PRINT_INTERNAL_STATE
            return CALL_NEXT_EVENT_HANDLER;
        }
    }

    return CALL_NEXT_EVENT_HANDLER;
}

uint8_t CombosHandler::resume_pending_combo_processing_release(KeyEvent event) {
    CombosBufferedKey* cbk = this->key_buffer.find(event.coords);
    // If the released key was never part of a combo, we won't find it in our
    // key_buffer. We must process it inside the next_event_handler.
    if (cbk == NULL) return CALL_NEXT_EVENT_HANDLER;

    if (cbk->part_of_active_combo) {
        this->release_active_combo_key(cbk, event.timestamp);
    } else {
        // We are releasing a key, that was part of a pending combo.
        // We must check if there is any fully pressed pending combo;
        // if so, activate it.
        int8_t fully_pressed_combo_index = -1;

        for (uint8_t i = 0; i < COMBOS_COUNT; i++) {
            ComboState* combo_state = COMBO_STATE(i);
            if (combo_state->is_activated()) continue;

            if (combo_state->is_fully_pressed()) {
                fully_pressed_combo_index = i;
            } else {
                combo_state->clear_state_and_flags();
            }
        }

        if (fully_pressed_combo_index >= 0) {
            this->activate_combo(fully_pressed_combo_index);
            ComboState* combo_state = COMBO_STATE(fully_pressed_combo_index);
            combo_state->remove_coords_from_state(event.coords);
        } else {
            // This handles clear/key_buffer.normalize
            this->abort_pending_combos_processing();
            PRINT_INTERNAL_STATE
            // Process the release key event with next_event_handler
            return CALL_NEXT_EVENT_HANDLER;
        }
    }

    cbk->clear();
    this->key_buffer.normalize();

    PRINT_INTERNAL_STATE

    return SKIP_NEXT_EVENT_HANDLER;
}

uint8_t CombosHandler::resume_pending_combo_processing_timeout(KeyEvent event) {
    if (this->pending_combos_start == 0) {
        return CALL_NEXT_EVENT_HANDLER;
    }

    // We are in the middle of a pending combo processing
    if (this->pending_combos_start + COMBO_ACTIVATION_TIMEOUT_MS > event.timestamp) {
        return SKIP_NEXT_EVENT_HANDLER;
    }

    int8_t fully_pressed_combo_index = -1;

    for (uint8_t i = 0; i < COMBOS_COUNT; i++) {
        ComboState* combo_state = COMBO_STATE(i);

        // Already activated combo is deactivated in EVENT_KEY_RELEASE
        if (combo_state->is_activated()) continue;

        if (combo_state->is_fully_pressed()) {
            fully_pressed_combo_index = i;
        } else {
            combo_state->clear_state_and_flags();
        }
    }

    if (fully_pressed_combo_index >= 0) {
        this->activate_combo(fully_pressed_combo_index);
    } else { // We have no fully pressed combos
        this->abort_pending_combos_processing();
    }

    PRINT_INTERNAL_STATE

    return CALL_NEXT_EVENT_HANDLER;
}

// End the current pending combo processing and send the buffered keys
// that are not part of some active combo to the next_event_handler
void CombosHandler::abort_pending_combos_processing() {
    for (uint8_t i = 0; i < this->key_buffer.count; i++) {
        CombosBufferedKey* cbk = this->key_buffer.get(i);
        if (cbk->part_of_active_combo) continue; // Part of some active combo
        if (cbk->timestamp == 0) continue; // Not pressed

        // Send the raw KeyCoords - the next_event_handler should decide what keys are those
        KeyEvent event = KeyEvent(EVENT_KEY_PRESS, cbk->coords, cbk->timestamp);
        this->next_event_handler->handle_key_event(event);

        cbk->clear();
    }

    // Clear all pending combos
    for (uint8_t i = 0; i < COMBOS_COUNT; i++) {
        ComboState* combo_state = COMBO_STATE(i);
        if (combo_state->is_activated()) continue;
        combo_state->clear_state_and_flags();
    }

    this->key_buffer.normalize();
    this->pending_combos_start = 0;
}

// Activate a combo and end the current pending combo processing
void CombosHandler::activate_combo(int8_t index) {
    // Mark ComboState as active
    ComboState* combo_state = COMBO_STATE(index);
    combo_state->set_activated();

    // Set the active_combo_index in the key_buffer
    for (uint8_t i = 0; i < this->key_buffer.count; i++) {
        CombosBufferedKey* cbk = this->key_buffer.get(i);
        if (cbk->part_of_active_combo) continue; // Part of other active combo
        if (cbk->timestamp == 0) continue; // Not pressed

        cbk->activate(index);
    }

    // Clear all pending combos
    for (uint8_t i = 0; i < COMBOS_COUNT; i++) {
        ComboState* combo_state = COMBO_STATE(i);
        if (combo_state->is_activated()) continue;
        combo_state->clear_state_and_flags();
    }

    this->pending_combos_start = 0;

    // Send the corresponding target_key event to next_event_handler
    KeyCoords coords = COMBO_TARGET_KEY_COORDS(index);
    KeyInfo key_info = KeyInfo(combo_state->target_key, coords);

    KeyEvent event = KeyEvent(EVENT_KEY_PRESS, coords, key_info, combo_state->timestamp);
    this->next_event_handler->handle_key_event(event);
}

void CombosHandler::release_active_combo_key(CombosBufferedKey* cbk, millisec timestamp) {
    ComboState* combo_state = COMBO_STATE(cbk->active_combo_index);
    combo_state->remove_coords_from_state(cbk->coords);

    if (combo_state->is_fully_released()) {
        KeyCoords coords = COMBO_TARGET_KEY_COORDS(cbk->active_combo_index);

        KeyEvent event = KeyEvent(EVENT_KEY_RELEASE, coords, timestamp);
        this->next_event_handler->handle_key_event(event);

        combo_state->clear_state_and_flags();
    }
}

void CombosHandler::print_internal_state(millisec now) {
    Serial.print("\nCombosHandler pending_combos_start:");
    Serial.print(this->pending_combos_start);
    Serial.print(" now-pending_combos_start:");
    Serial.print(now - this->pending_combos_start);
    Serial.print(" last_passthrough_event:");
    Serial.print(this->last_passthrough_event);
    Serial.print(" now-last_passthrough_event:");
    Serial.print(now - this->last_passthrough_event);

    this->key_buffer.print_internal_state(now);

    Serial.print("ComboState\n");
    for (uint8_t i = 0; i < COMBOS_COUNT; i++) {
        ComboState* combo_state = COMBO_STATE(i);
        combo_state->print_internal_state(i, now);
    }
}
