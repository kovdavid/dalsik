#include "matrix.h"
#include "HID.h"
#include "dalsik_hid_desc.h"
#include "keymap.h"
#include "array_utils.h"
#include "dalsik.h"

extern const uint8_t KEYBOARD_HID_DESC[] PROGMEM;

MasterReport::MasterReport(KeyMap* keymap) {
    static HIDSubDescriptor node(KEYBOARD_HID_DESC, sizeof(KEYBOARD_HID_DESC));
    HID().AppendDescriptor(&node);

    this->keymap = keymap;
    this->clear();

    for (uint8_t i = 0; i < MAX_TAPDANCE_KEYS; i++) {
        memset(&(this->tapdance_state[i]), 0, sizeof(TapDanceState));
    }
    this->active_tapdance_key_count = 0;
    this->last_tapdance_press_ts = 0;
    this->last_tapdance_index = 0;
}

void MasterReport::clear() {
    memset(&(this->base_hid_report), 0, sizeof(BaseHIDReport));
    memset(&(this->system_hid_report), 0, sizeof(SystemHIDReport));
    memset(&(this->multimedia_hid_report), 0, sizeof(MultimediaHIDReport));
    memset(&(this->dual_key_state), 0, sizeof(DualKeyState));
    memset(&(this->dual_layer_key_state), 0, sizeof(DualKeyState));
    memset(&(this->hold_or_toggle_state), 0, sizeof(LayerHoldOrToggleState));

    this->num_keys_pressed = 0;

    this->base_keys_pressed = 0;
    this->base_hid_report_changed = 1;
    this->system_keys_pressed = 0;
    this->system_hid_report_changed = 1;
    this->multimedia_keys_pressed = 0;
    this->multimedia_hid_report_changed = 1;

    this->keymap->clear();
}

void MasterReport::handle_master_changed_key(ChangedKeyCoords coords) {
    KeyInfo key_info = this->keymap->get_master_key(coords.row, coords.col);
    if (coords.type == EVENT_KEY_PRESS) {
        this->handle_key_press(key_info);
    } else if (coords.type == EVENT_KEY_RELEASE) {
        this->handle_key_release(key_info);
    }
}

inline void MasterReport::handle_slave_changed_key(ChangedKeyCoords coords) {
    KeyInfo key_info = this->keymap->get_slave_key(coords.row, coords.col);
    if (coords.type == EVENT_KEY_PRESS) {
        this->handle_key_press(key_info);
    } else if (coords.type == EVENT_KEY_RELEASE) {
        this->handle_key_release(key_info);
    }
}

inline void MasterReport::handle_key_press(KeyInfo key_info) {
    this->num_keys_pressed++;
    this->press(key_info);
    this->send_hid_report();
}

void MasterReport::handle_key_release(KeyInfo key_info) {
    this->num_keys_pressed--;
    this->release(key_info);
    if (this->num_keys_pressed == 0) {
        this->release_all_keys_hook_for_tapdance_keys();
        this->clear();
    }

    this->send_hid_report();
}

void MasterReport::press(KeyInfo key_info) {
#if DEBUG
    Serial.print("press:");
    Serial.print(KeyMap::key_type_to_string(key_info));
    Serial.print("|");
    Serial.print(key_info.key, HEX);
    Serial.print("\n");
#endif

    this->press_hook_for_dual_keys(&key_info);
    this->press_hook_for_layer_hold_or_toggle(&key_info);
    this->press_hook_for_tapdance_keys(key_info);

    if (key_info.type == KEY_NORMAL) {
        this->press_normal_key(key_info);
    } else if (key_info.type == KEY_LAYER_PRESS) {
        this->press_layer_key(key_info.key);
    } else if (key_info.type == KEY_LAYER_TOGGLE) {
        this->press_toggle_layer_key(key_info.key);
    } else if (key_info.type == KEY_LAYER_HOLD_OR_TOGGLE) {
        this->press_layer_hold_or_toggle(key_info);
    } else if (key_info.type == KEY_TAPDANCE) {
        this->press_tapdance_key(key_info);
    } else if (key_info.type == KEY_SYSTEM)  {
        this->press_system_key(key_info);
    } else if (KeyMap::is_dual_key(key_info)) {
        this->press_dual_key(key_info);
    } else if (KeyMap::is_dual_single_key(key_info)) {
        this->press_dual_single_key(key_info);
    } else if (KeyMap::is_dual_layer_key(key_info)) {
        this->press_dual_layer_key(key_info);
    } else if (KeyMap::is_dual_layer_single_key(key_info)) {
        this->press_dual_layer_single_key(key_info);
    } else if (KeyMap::is_multimedia_key(key_info)) {
        this->press_multimedia_key(key_info);
    } else if (KeyMap::is_key_with_mod(key_info)) {
        this->press_key_with_mod(key_info);
    }
}

void MasterReport::release(KeyInfo key_info) {
#if DEBUG
    Serial.print("release:");
    Serial.print(KeyMap::key_type_to_string(key_info));
    Serial.print("|");
    Serial.print(key_info.key, HEX);
    Serial.print("\n");
#endif

    if (key_info.type == KEY_NORMAL) {
        this->release_normal_key(key_info);
    } else if (key_info.type == KEY_LAYER_PRESS) {
        this->release_layer_key(key_info.key);
    } else if (key_info.type == KEY_LAYER_TOGGLE) {
        // do nothing; toggle_layer key has only effect on press
    } else if (key_info.type == KEY_LAYER_HOLD_OR_TOGGLE) {
        this->release_layer_hold_or_toggle(key_info);
    } else if (key_info.type == KEY_TAPDANCE) {
        this->release_tapdance_key(key_info);
    } else if (key_info.type == KEY_SYSTEM)  {
        this->release_system_key(key_info);
    } else if (KeyMap::is_dual_key(key_info) || KeyMap::is_dual_single_key(key_info)) {
        this->release_dual_key(key_info);
    } else if (KeyMap::is_dual_layer_key(key_info) || KeyMap::is_dual_layer_single_key(key_info)) {
        this->release_dual_layer_key(key_info);
    } else if (KeyMap::is_multimedia_key(key_info)) {
        this->release_multimedia_key(key_info);
    } else if (KeyMap::is_key_with_mod(key_info)) {
        this->release_key_with_mod(key_info);
    }
}

inline void MasterReport::press_normal_key(KeyInfo key_info) {
    uint8_t key = key_info.key;

    if (key >= 0xE0 && key <= 0xE7) { // modifier
        // For 'Left Shift' 0xE1 bitmask is 0x0000_0010
        uint8_t modifier_bit = key & 0x0F;
        uint8_t bitmask = 1 << modifier_bit;
        this->base_hid_report.modifiers |= bitmask;
    } else { // key
        append_uniq_to_uint8_array(this->base_hid_report.keys, BASE_HID_REPORT_KEYS, key);
    }

    this->base_hid_report_changed = 1;
    this->base_keys_pressed++;
}

inline void MasterReport::release_normal_key(KeyInfo key_info) {
    uint8_t key = key_info.key;

    if (key >= 0xE0 && key <= 0xE7) { // modifier
        // For 'Left Shift' 0xE1 bitmask is 0x1111_1101
        uint8_t modifier_bit = key & 0x0F;
        uint8_t bitmask = (1 << modifier_bit) ^ 0xFF;
        this->base_hid_report.modifiers &= bitmask;
    } else { // key
        remove_uniq_from_uint8_array(this->base_hid_report.keys, BASE_HID_REPORT_KEYS, key);
    }

    this->base_hid_report_changed = 1;
    this->base_keys_pressed--;
}

inline void MasterReport::press_layer_key(uint8_t layer) {
    this->keymap->set_layer(layer);
}

inline void MasterReport::release_layer_key(uint8_t layer) {
    this->keymap->remove_layer(layer);
}

inline void MasterReport::press_toggle_layer_key(uint8_t layer) {
    this->keymap->toggle_layer(layer);
}

inline void MasterReport::press_dual_key(KeyInfo key_info) {
    // Old dual_key (if any) is already activated as a modifier in the press_hook_for_dual_keys
    this->dual_key_state.last_press_ts = millis();
    this->dual_key_state.key_info = key_info;
    this->dual_key_state.mode = DUAL_MODE_PENDING;

    if (!LAZY_DUAL_KEYS && this->num_keys_pressed > 1) {
        this->dual_key_state.mode = DUAL_MODE_HOLD_MODIFIER;
        uint8_t modifier = KeyMap::get_dual_key_modifier(key_info);
        KeyInfo ki = KeyMap::init_key_info(KEY_NORMAL, modifier, key_info.row, key_info.col);
        this->press_normal_key(ki);
    }
}

inline void MasterReport::press_dual_single_key(KeyInfo key_info) {
    this->dual_key_state.last_press_ts = millis();
    this->dual_key_state.key_info = key_info;

    if (this->num_keys_pressed > 1) {
        this->dual_key_state.mode = DUAL_MODE_PRESS_KEY;
        this->press_normal_key(key_info);
        this->send_hid_report();
    } else {
        this->dual_key_state.mode = DUAL_MODE_PENDING;
    }
}

inline void MasterReport::release_dual_key(KeyInfo key_info) {
    if (KeyMap::key_info_compare(key_info, this->dual_key_state.key_info) == 0) {
        if (this->dual_key_state.mode == DUAL_MODE_HOLD_MODIFIER) {
            uint8_t modifier = KeyMap::get_dual_key_modifier(key_info);
            KeyInfo ki = KeyMap::init_key_info(KEY_NORMAL, modifier, key_info.row, key_info.col);
            this->release_normal_key(ki);
        } else if (this->dual_key_state.mode == DUAL_MODE_PRESS_KEY) {
            KeyInfo ki = KeyMap::init_key_info(KEY_NORMAL, key_info.key, key_info.row, key_info.col);
            this->release_normal_key(ki);
        } else {
            KeyInfo ki = KeyMap::init_key_info(KEY_NORMAL, key_info.key, key_info.row, key_info.col);
            this->press_normal_key(ki);
            this->send_base_hid_report();
            this->release_normal_key(ki);
        }

        memset(&(this->dual_key_state), 0, sizeof(DualKeyState));
    } else {
        // There were more dual_keys pressed, this one is not the first
        uint8_t modifier = KeyMap::get_dual_key_modifier(key_info);
        KeyInfo ki = KeyMap::init_key_info(KEY_NORMAL, modifier, key_info.row, key_info.col);
        this->release_normal_key(ki);
    }
}

inline void MasterReport::press_dual_layer_key(KeyInfo key_info) {
    // Old dual_layer_key (if any) is already activated as a layer_press in the press_hook_for_dual_keys
    this->dual_layer_key_state.last_press_ts = millis();
    this->dual_layer_key_state.key_info = key_info;
    this->dual_layer_key_state.mode = DUAL_MODE_PENDING;

    if (!LAZY_DUAL_KEYS && this->num_keys_pressed > 1) {
        this->dual_layer_key_state.mode = DUAL_MODE_HOLD_LAYER;
        uint8_t layer = KeyMap::get_dual_layer_key_layer(key_info);
        this->press_layer_key(layer);
    }
}

inline void MasterReport::press_dual_layer_single_key(KeyInfo key_info) {
    this->dual_layer_key_state.last_press_ts = millis();
    this->dual_layer_key_state.key_info = key_info;

    if (this->num_keys_pressed > 1) {
        this->dual_layer_key_state.mode = DUAL_MODE_PRESS_KEY;
        this->press_normal_key(key_info);
        this->send_hid_report();
    } else {
        this->dual_layer_key_state.mode = DUAL_MODE_PENDING;
    }
}

inline void MasterReport::release_dual_layer_key(KeyInfo key_info) {
    if (KeyMap::key_info_compare(key_info, this->dual_layer_key_state.key_info) == 0) {
        if (this->dual_layer_key_state.mode == DUAL_MODE_HOLD_LAYER) {
            uint8_t layer = KeyMap::get_dual_layer_key_layer(key_info);
            this->release_layer_key(layer);
        } else if (this->dual_layer_key_state.mode == DUAL_MODE_PRESS_KEY) {
            KeyInfo ki = KeyMap::init_key_info(KEY_LAYER_PRESS, key_info.key, key_info.row, key_info.col);
            this->release_normal_key(ki);
        } else {
            KeyInfo ki = KeyMap::init_key_info(KEY_NORMAL, key_info.key, key_info.row, key_info.col);
            this->press_normal_key(ki);
            this->send_base_hid_report();
            this->release_normal_key(ki);
        }

        memset(&(this->dual_layer_key_state), 0, sizeof(DualKeyState));
    } else {
        // There were more dual_layer_keys pressed, this one is not the first
        uint8_t layer = KeyMap::get_dual_layer_key_layer(key_info);
        this->release_layer_key(layer);
    }
}

inline void MasterReport::press_multimedia_key(KeyInfo key_info) {
    this->multimedia_hid_report.key = key_info.key;
    if (key_info.type == KEY_MULTIMEDIA_2) {
        this->multimedia_hid_report.prefix = 0x02;
    } else if (key_info.type == KEY_MULTIMEDIA_1) {
        this->multimedia_hid_report.prefix = 0x01;
    } else {
        this->multimedia_hid_report.prefix = 0x00;
    }
    this->multimedia_hid_report_changed = 1;
    this->multimedia_keys_pressed++;
}

inline void MasterReport::release_multimedia_key(KeyInfo key_info) {
    uint8_t key = key_info.key;
    uint8_t prefix = 0x00;
    if (key_info.type == KEY_MULTIMEDIA_2) {
        prefix = 0x02;
    } else if (key_info.type == KEY_MULTIMEDIA_1) {
        prefix = 0x01;
    }

    if (
        this->multimedia_hid_report.key == key
        && this->multimedia_hid_report.prefix == prefix
    ) {
        memset(&(this->multimedia_hid_report), 0, sizeof(MultimediaHIDReport));
        this->multimedia_hid_report_changed = 1;
        this->multimedia_keys_pressed--;
    }
}

inline void MasterReport::press_hook_for_dual_keys(KeyInfo* current_ki) {
    if (this->dual_key_state.mode == DUAL_MODE_PENDING) {
        this->dual_key_state.mode = DUAL_MODE_HOLD_MODIFIER;
        uint8_t modifier = KeyMap::get_dual_key_modifier(this->dual_key_state.key_info);
        this->press_normal_key(KeyMap::init_key_info_without_coords(KEY_NORMAL, modifier));
        this->send_hid_report();
    }
    if (this->dual_layer_key_state.mode == DUAL_MODE_PENDING) {
        this->dual_layer_key_state.mode = DUAL_MODE_HOLD_LAYER;
        uint8_t layer = KeyMap::get_dual_layer_key_layer(this->dual_layer_key_state.key_info);
        this->press_layer_key(layer);

        this->keymap->reload_key_info_by_row_col(current_ki);
    }
}

inline void MasterReport::press_layer_hold_or_toggle(KeyInfo key_info) {
    if (this->hold_or_toggle_state.mode == HOLD_OR_TOGGLE_NOT_PRESSED) {
        this->hold_or_toggle_state.key_info = key_info;
        if (this->num_keys_pressed > 1) {
            this->hold_or_toggle_state.mode = HOLD_OR_TOGGLE_HOLD_LAYER;
            this->keymap->set_layer(key_info.key);
        } else {
            this->hold_or_toggle_state.mode = HOLD_OR_TOGGLE_PENDING;
        }
    } else {
        this->keymap->set_layer(key_info.key);
    }
}

inline void MasterReport::release_layer_hold_or_toggle(KeyInfo key_info) {
    if (KeyMap::key_info_compare(key_info, this->hold_or_toggle_state.key_info) == 0) {
        if (this->hold_or_toggle_state.mode == HOLD_OR_TOGGLE_HOLD_LAYER) {
            this->keymap->remove_layer(key_info.key);
        } else {
            this->keymap->toggle_layer(key_info.key);
        }

        memset(&(this->hold_or_toggle_state), 0, sizeof(LayerHoldOrToggleState));
    } else {
        this->keymap->remove_layer(key_info.key);
    }
}

inline void MasterReport::press_system_key(KeyInfo key_info) {
    this->system_hid_report.key = key_info.key;
    this->system_hid_report_changed = 1;
    this->system_keys_pressed++;
}

inline void MasterReport::release_system_key(KeyInfo key_info) {
    if (this->system_hid_report.key == key_info.key) {
        this->system_hid_report.key = 0x00;
        this->system_hid_report_changed = 1;
        this->system_keys_pressed--;
    }
}

inline void MasterReport::press_hook_for_layer_hold_or_toggle(KeyInfo* current_ki) {
    if (this->hold_or_toggle_state.mode == HOLD_OR_TOGGLE_PENDING) {
        this->hold_or_toggle_state.mode = HOLD_OR_TOGGLE_HOLD_LAYER;
        this->keymap->set_layer(this->hold_or_toggle_state.key_info.key);
        this->keymap->reload_key_info_by_row_col(current_ki);
    }
}

inline void MasterReport::press_key_with_mod(KeyInfo key_info) {
    uint8_t modifier = KeyMap::get_key_with_mod_modifier(key_info);
    this->press(KeyMap::init_key_info(KEY_NORMAL, modifier, key_info.row, key_info.col));
    this->press(KeyMap::init_key_info(KEY_NORMAL, key_info.key, key_info.row, key_info.col));
}

inline void MasterReport::release_key_with_mod(KeyInfo key_info) {
    uint8_t modifier = KeyMap::get_key_with_mod_modifier(key_info);
    this->release(KeyMap::init_key_info(KEY_NORMAL, modifier, key_info.row, key_info.col));
    this->release(KeyMap::init_key_info(KEY_NORMAL, key_info.key, key_info.row, key_info.col));
}

inline void MasterReport::press_hook_for_tapdance_keys(KeyInfo key_info) {
    if (key_info.type == KEY_TAPDANCE && key_info.key == this->last_tapdance_index) {
        return;
    }
    uint8_t last_index = this->last_tapdance_index;
    if (this->tapdance_state[last_index].tap_count == 0) {
        return;
    }
    if (this->tapdance_state[last_index].key_reported == 0) {
        this->activate_tapdance(last_index);
    }
}

// Handle stuck tapdance keys
inline void MasterReport::release_all_keys_hook_for_tapdance_keys() {
    for (uint8_t i = 0; i < MAX_TAPDANCE_KEYS; i++) {
        if (this->tapdance_state[i].key_pressed == 1) {
            KeyInfo ki = KeyMap::init_key_info_without_coords(KEY_TAPDANCE, i);
            this->release(ki);
        }
    }
    this->send_hid_report();
}

inline void MasterReport::activate_tapdance(uint8_t index) {
    KeyInfo key_info = this->keymap->get_tapdance_key(
        index, this->tapdance_state[index].tap_count
    );

    this->tapdance_state[index].key_reported = 1;
    this->press(key_info);
    this->send_hid_report();

    if (this->tapdance_state[index].key_pressed == 0) {
        memset(&(this->tapdance_state[index]), 0, sizeof(TapDanceState));
        this->active_tapdance_key_count--;
        this->release(key_info);
        this->send_hid_report();
    }
}

inline void MasterReport::press_tapdance_key(KeyInfo key_info) {
    uint8_t index = key_info.key;

    if (index >= MAX_TAPDANCE_KEYS) {
        return;
    }

    this->last_tapdance_index = index;
    this->last_tapdance_press_ts = millis();

    if (this->tapdance_state[index].tap_count == 0) {
        this->tapdance_state[index] = TapDanceState {
            .key_reported = 0,
            .key_pressed = 1,
            .tap_count = 1,
        };
        this->active_tapdance_key_count++;
        return;
    }

    this->tapdance_state[index].tap_count++;
    this->tapdance_state[index].key_pressed = 1;

    if (this->tapdance_state[index].tap_count >= MAX_TAPDANCE_TAPS) {
        this->activate_tapdance(index);
    }
}

inline void MasterReport::release_tapdance_key(KeyInfo key_info) {
    uint8_t index = key_info.key;

    if (index >= MAX_TAPDANCE_KEYS) {
        return;
    }

    this->tapdance_state[index].key_pressed = 0;

    if (this->tapdance_state[index].key_reported == 1) {
        KeyInfo key_info = this->keymap->get_tapdance_key(
            index, this->tapdance_state[index].tap_count
        );
        memset(&(this->tapdance_state[index]), 0, sizeof(TapDanceState));
        this->active_tapdance_key_count--;
        this->release(key_info);
    }
}

void MasterReport::key_timeout_check() {
    TapDanceState tapdance_state = this->tapdance_state[this->last_tapdance_index];
    if (
        tapdance_state.tap_count != 0
        && tapdance_state.key_reported == 0
        && this->last_tapdance_press_ts + TAPDANCE_TIMEOUT_MS < millis()
    ) {
        this->activate_tapdance(this->last_tapdance_index);
    }

    this->dual_key_timeout_check(&(this->dual_key_state));
    this->dual_key_timeout_check(&(this->dual_layer_key_state));
}

inline void MasterReport::dual_key_timeout_check(DualKeyState* state) {
#if DUAL_MODE_TIMEOUT_MS
    if (
        state->mode == DUAL_MODE_PENDING
        &&
        state->last_press_ts + DUAL_MODE_TIMEOUT_MS < millis()
    ) {
        state->mode = DUAL_MODE_PRESS_KEY;
        this->press_normal_key(state->key_info);
        this->send_hid_report();
    }
#endif
}

void MasterReport::print_base_report_to_serial() {
    Serial.print(F("MasterReport[BASE]:"));
    Serial.print(this->base_hid_report.modifiers, HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.reserved, HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.keys[0], HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.keys[1], HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.keys[2], HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.keys[3], HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.keys[4], HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.keys[5], HEX);
    Serial.print(F("\n"));
}

void MasterReport::print_system_report_to_serial() {
    Serial.print(F("MasterReport[SYSTEM]:"));
    Serial.print(this->system_hid_report.key, HEX);
    Serial.print(F("\n"));
}

void MasterReport::print_multimedia_report_to_serial() {
    Serial.print(F("MasterReport[MULTIMEDIA]:"));
    Serial.print(this->multimedia_hid_report.prefix, HEX);
    Serial.print(F("|"));
    Serial.print(this->multimedia_hid_report.key, HEX);
    Serial.print(F("\n"));
}

void MasterReport::send_hid_report() {
    if (this->base_hid_report_changed == 1) {
        this->send_base_hid_report();
        this->base_hid_report_changed = 0;
    }
    if (this->system_hid_report_changed == 1) {
        this->send_system_hid_report();
        this->system_hid_report_changed = 0;
    }
    if (this->multimedia_hid_report_changed == 1) {
        this->send_multimedia_hid_report();
        this->multimedia_hid_report_changed = 0;
    }
}

void MasterReport::send_base_hid_report() {
#if DEBUG_KEYREPORT_BASE
    this->print_base_report_to_serial();
#endif
    void *report = &(this->base_hid_report);
    HID().SendReport(BASE_KEYBOARD_REPORT_ID, report, sizeof(BaseHIDReport));
}

void MasterReport::send_system_hid_report() {
#if DEBUG_KEYREPORT_SYSTEM
    this->print_system_report_to_serial();
#endif
    void *report = &(this->system_hid_report);
    HID().SendReport(SYSTEM_KEYBOARD_REPORT_ID, report, sizeof(SystemHIDReport));
}

void MasterReport::send_multimedia_hid_report() {
#if DEBUG_KEYREPORT_MULTIMEDIA
    this->print_multimedia_report_to_serial();
#endif
    void *report = &(this->multimedia_hid_report);
    HID().SendReport(MULTIMEDIA_KEYBOARD_REPORT_ID, report, sizeof(MultimediaHIDReport));
}
