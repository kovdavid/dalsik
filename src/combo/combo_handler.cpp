#include "Arduino.h"
#include "combo.h"
#include "combo_handler.h"
#include "extended_key_event.h"
#include "tapdance_handler.h"

#define SKIP_NEXT_EVENT_HANDLER 0x01
#define CALL_NEXT_EVENT_HANDLER 0x02

#if DEBUG_COMBO_HANDLER_STATE
#define PRINT_INTERNAL_STATE this->print_internal_state(event.timestamp);
#else
#define PRINT_INTERNAL_STATE
#endif

ComboHandler::ComboHandler(TapDanceHandler *next_event_handler)
    : next_event_handler(next_event_handler)
    , state({})
{}

uint8_t ComboHandler::handle_key_event(ExtendedKeyEvent event) {
    uint8_t action = CALL_NEXT_EVENT_HANDLER;

    if (combos_count > 0) {
        if (this->state.pending_combo_start > 0) {
            action = this->resume_pending_combo_processing(event);
        } else {
            action = this->no_pending_combo_processing(event);
        }
    }

    if (action == CALL_NEXT_EVENT_HANDLER) {
        this->next_event_handler->handle_key_event(event);
    }

    return action;
}

// If the event key is part of a combo, then start a pending combo processing.
// Otherwise pass the event to next_event_handler.
uint8_t ComboHandler::no_pending_combo_processing(ExtendedKeyEvent event) {
    if (event.type == EVENT_TIMEOUT) {
        // No combo is active/pending; just pass the execution to next_event_handler
        return CALL_NEXT_EVENT_HANDLER;
    }

    if (event.type == EVENT_KEY_RELEASE) {
        return this->no_pending_combo_processing_release(event);
    }

    // We need to have COMBO_START_THRESHOLD_MS milliseconds without any events
    // to start combo processing. This is to prevent accidental combo firing
    // when typing
    if (this->state.last_non_combo_press + COMBO_START_THRESHOLD_MS > event.timestamp) {
        this->state.last_non_combo_press = event.timestamp;
        return CALL_NEXT_EVENT_HANDLER;
    }

    return this->no_pending_combo_processing_press(event);
}

// Even if there is no pending Combo processing, we need to handle release events for already activate combos
uint8_t ComboHandler::no_pending_combo_processing_release(ExtendedKeyEvent event) {
    uint8_t action = CALL_NEXT_EVENT_HANDLER;

    HeldUpKey* hkey = this->state.held_up_keys.find_by_coords(event.coords);
    if (hkey == NULL) {
        // The key is not part of an active combo (if it were, it would be in HeldUpKeys),
        // nor do we have a pending combo processing - nothing to do here
        return action;
    }

    if (hkey->part_of_active_combo) {
        this->release_active_combo_key(hkey, event.timestamp);
        action = SKIP_NEXT_EVENT_HANDLER;
        PRINT_INTERNAL_STATE
    }

    this->state.held_up_keys.remove(hkey);

    return action;
}

// Start a pending combo processing if necessary
uint8_t ComboHandler::no_pending_combo_processing_press(ExtendedKeyEvent event) {
    PressScanResult scan_result = this->scan_combos_on_press_event(event);

    if (scan_result.affected_combos_count > 0) {
        // If there was at least one affected combo by our press event, we start a pending
        // combo processing. The current event needs to be put into held_up_keys and we need
        // to halt its further processing until the pending combo is resolved.
        this->state.held_up_keys.add(event.coords, event.timestamp);
        this->state.pending_combo_start = event.timestamp;
        PRINT_INTERNAL_STATE
        return SKIP_NEXT_EVENT_HANDLER;
    } else {
        // We did not start a pending combo processing, so we need to clear all the disabled
        // flags that were set in scan_combos_on_press_event.
        if (scan_result.disabled_combos_count > 0) {
            for (uint8_t i = 0; i < combos_count; i++) {
                Combo* combo = COMBO_AT(i);
                if (combo->is_activated()) continue;

                combo->clear_disabled_flag();
            }

            PRINT_INTERNAL_STATE
        }

        this->state.last_non_combo_press = event.timestamp;

        return CALL_NEXT_EVENT_HANDLER;
    }
}

// Scan the combos and mark the affected ones (those that contain the coords from
// the event) and disable the rest
PressScanResult ComboHandler::scan_combos_on_press_event(ExtendedKeyEvent event) {
    PressScanResult result;

    for (uint8_t i = 0; i < combos_count; i++) {
        Combo* combo = COMBO_AT(i);
        if (combo->is_activated()) continue;
        if (combo->is_disabled()) continue;

        if (combo->update_state_if_affected(event.coords)) {
            // This combo contains our event.coords
            result.affected_combos_count++;

            if (combo->is_fully_pressed()) {
                result.fully_pressed_combo_index = i;
            }
        } else {
            result.disabled_combos_count++;
            combo->set_disabled_flag();
        }
    }

    return result;
}

uint8_t ComboHandler::resume_pending_combo_processing(ExtendedKeyEvent event) {
    if (event.type == EVENT_KEY_PRESS) {
        return this->resume_pending_combo_processing_press(event);
    } else if (event.type == EVENT_KEY_RELEASE) {
        return this->resume_pending_combo_processing_release(event);
    } else {
        return this->resume_pending_combo_processing_timeout(event);
    }
}

uint8_t ComboHandler::resume_pending_combo_processing_press(ExtendedKeyEvent event) {
    PressScanResult scan_result = this->scan_combos_on_press_event(event);

    if (scan_result.affected_combos_count > 0) {
        // We got a press event for a Combo key, so we continue with our pending processing
        this->state.held_up_keys.add(event.coords, event.timestamp);

        if (scan_result.affected_combos_count == 1 && scan_result.fully_pressed_combo_index >= 0) {
            // If only 1 pending combo remains pending (that is also fully pressed),
            // we can go ahead and activate it.
            this->activate_combo(scan_result.fully_pressed_combo_index);
        } else {
            // There are either more than 1 affected pending combos, or the
            // only pending combo is not fully pressed yet.
        }

        PRINT_INTERNAL_STATE

        return SKIP_NEXT_EVENT_HANDLER;
    } else if (scan_result.disabled_combos_count > 0) {
        // So there were no affected pending combos, but we've disabled some previously pending combos.
        // Because they were pending before, one of them may be fully pressed, which we will activate.
        //
        // Example:
        // We have 2 Combos: [KeyCoords(0,0), KeyCoords(1,1)] and [KeyCoords(0,0), KeyCoords(1,1), KeyCoords(2,2)]
        // We press (0,0) then (1,1). Both our combos are still pending. At this point we press (3,3) (which is
        // not part of any Combo), so we activate our first Combo ([KeyCoords(0,0), KeyCoords(1,1)]), because it
        // was fully pressed.

        if (scan_result.fully_pressed_combo_index >= 0) {
            this->activate_combo(scan_result.fully_pressed_combo_index);
            PRINT_INTERNAL_STATE
            // The newly pressed key could start a new pending combo processing,
            // so we call handle_key_event again
            return this->handle_key_event(event);
        } else {
            this->abort_pending_combos_processing();
            PRINT_INTERNAL_STATE
            return CALL_NEXT_EVENT_HANDLER;
        }
    }

    return CALL_NEXT_EVENT_HANDLER;
}

int8_t ComboHandler::find_fully_pressed_pending_combo() {
    int8_t fully_pressed_combo_index = -1;

    for (uint8_t i = 0; i < combos_count; i++) {
        Combo* combo = COMBO_AT(i);

        // Already activated combos are deactivated in the release handler
        if (combo->is_activated()) continue;

        if (combo->is_fully_pressed()) {
            fully_pressed_combo_index = i;
        }
    }

    return fully_pressed_combo_index;
}

// We are resolving our pending combo processing one way or the other.
// We must check if there is any fully pressed pending combo; if so, activate it.
// Example:
// We have 2 Combos: [KeyCoords(0,0), KeyCoords(1,1)] and [KeyCoords(0,0), KeyCoords(1,1), KeyCoords(2,2)]
// We press (0,0) then (1,1). Both our combos are still pending. If at this point we release a key (it could be
// one of a pending combo key, like (1,1) or a different previously pressed key), we will activate the first combo.
// If the released key were (1,1), we need to handle it correctly and release it from `Combo::pressed_coords`.
uint8_t ComboHandler::resume_pending_combo_processing_release(ExtendedKeyEvent event) {
    int8_t fully_pressed_combo_index = this->find_fully_pressed_pending_combo();

    if (fully_pressed_combo_index >= 0) {
        this->activate_combo(fully_pressed_combo_index);
    } else {
        this->abort_pending_combos_processing();
    }

    HeldUpKey* hkey = this->state.held_up_keys.find_by_coords(event.coords);
    if (hkey != NULL) {
        // The released key is still in HeldUpKeys.
        // We called either activate_combo or abort_pending_combos_processing, both of which end the pending
        // combo processing. That means that this key must be part of some active combo.

        this->release_active_combo_key(hkey, event.timestamp);
        this->state.held_up_keys.remove(hkey);

        PRINT_INTERNAL_STATE

        return SKIP_NEXT_EVENT_HANDLER;
    } else {
        PRINT_INTERNAL_STATE

        return CALL_NEXT_EVENT_HANDLER;
    }
}

uint8_t ComboHandler::resume_pending_combo_processing_timeout(ExtendedKeyEvent event) {
    if (this->state.pending_combo_start + COMBO_PROCESSING_LIMIT_MS > event.timestamp) {
        return SKIP_NEXT_EVENT_HANDLER;
    }

    // We are going to resolve the pending combo processing either by activating a combo or aborting

    // If we encounter a not active Combo that is fully pressed, we are going to activate it.
    // An example of when that can happen:
    // If we have 2 Combos: [KeyCoords(0,0), KeyCoords(1,1)] and [KeyCoords(0,0), KeyCoords(1,1), KeyCoords(2,2)]
    // After pressing (0,0) and (1,1) we cannot activate the first combo yet, because we might press (2,2) in the necessary
    // future. Such situation is resolved either by pressing (2,2), releasing a key, or by this timeout handler
    int8_t fully_pressed_combo_index = -1;

    for (uint8_t i = 0; i < combos_count; i++) {
        Combo* combo = COMBO_AT(i);

        // Already activated combos are deactivated in the release handler
        if (combo->is_activated()) continue;

        if (combo->is_fully_pressed()) {
            fully_pressed_combo_index = i;
        }
    }

    if (fully_pressed_combo_index >= 0) {
        this->activate_combo(fully_pressed_combo_index);
    } else {
        // We don't have any fully pressed Combos
        this->abort_pending_combos_processing();
    }

    PRINT_INTERNAL_STATE

    return CALL_NEXT_EVENT_HANDLER;
}

// Activate a combo and end the current pending combo processing
void ComboHandler::activate_combo(int8_t index) {
    // Mark Combo as activated
    Combo* combo = COMBO_AT(index);
    combo->set_activated_flag();

    // Set the active_combo_index in held_up_keys
    for (uint8_t i = 0; i < this->state.held_up_keys.count; i++) {
        HeldUpKey* hkey = this->state.held_up_keys.get_by_index(i);
        if (hkey->part_of_active_combo) continue; // Part of other active combo
        if (hkey->timestamp == 0) continue; // Not pressed

        hkey->activate(index);
    }

    // Clear all pending combos
    for (uint8_t i = 0; i < combos_count; i++) {
        Combo* combo = COMBO_AT(i);
        if (combo->is_activated()) continue;
        combo->clear_state();
    }

    // Send the corresponding target_key event to next_event_handler
    KeyCoords coords = COMBO_TARGET_KEY_COORDS(index);
    Key key = Key(combo->target_key, coords);

    ExtendedKeyEvent event = ExtendedKeyEvent(EVENT_KEY_PRESS, coords, key, this->state.pending_combo_start);
    this->next_event_handler->handle_key_event(event);

    // Deactivate pending Combo processing
    this->state.pending_combo_start = 0;
}

void ComboHandler::abort_pending_combos_processing() {
    // Send the HeldUpKeys to next_event_handler for processing
    for (uint8_t i = 0; i < this->state.held_up_keys.count; i++) {
        HeldUpKey* hkey = this->state.held_up_keys.get_by_index(i);
        if (hkey->part_of_active_combo) continue;
        if (hkey->timestamp == 0) continue;

        ExtendedKeyEvent event = ExtendedKeyEvent(EVENT_KEY_PRESS, hkey->coords, hkey->timestamp);

        this->next_event_handler->handle_key_event(event);

        hkey->clear();
    }

    this->state.held_up_keys.repack_keys();

    for (uint8_t i = 0; i < combos_count; i++) {
        Combo* combo = COMBO_AT(i);
        if (combo->is_activated()) continue;
        combo->clear_state();
    }

    this->state.pending_combo_start = 0;
}

void ComboHandler::release_active_combo_key(HeldUpKey* hkey, millisec timestamp) {
    Combo* combo = COMBO_AT(hkey->active_combo_index);
    combo->release_coords(hkey->coords);

    if (combo->is_fully_released()) {
        KeyCoords coords = COMBO_TARGET_KEY_COORDS(hkey->active_combo_index);

        ExtendedKeyEvent event = ExtendedKeyEvent(EVENT_KEY_RELEASE, coords, timestamp);

        this->next_event_handler->handle_key_event(event);

        combo->clear_state();
    }
}

void ComboHandler::print_internal_state(millisec now) {
    Serial.print("\nComboHandler pending_combo_start:");
    Serial.print(this->state.pending_combo_start);
    Serial.print(" now-pending_combo_start:");
    Serial.print(now - this->state.pending_combo_start);
    Serial.print(" last_passthrough_event:");
    Serial.print(this->state.last_non_combo_press);
    Serial.print(" now-last_passthrough_event:");
    Serial.print(now - this->state.last_non_combo_press);

    this->state.held_up_keys.print_internal_state(now);

    Serial.print("Combo\n");
    for (uint8_t i = 0; i < combos_count; i++) {
        COMBO_AT(i)->print_internal_state(i);
    }
}
