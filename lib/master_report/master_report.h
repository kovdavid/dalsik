#pragma once

#include "matrix.h"
#include "keymap.h"
#include "dalsik.h"
#include "dalsik_hid.h"

#define STATE_NOT_PROCESSED 0x00
#define STATE_PENDING       0x01
#define STATE_PRIMARY_KEY   0x02
#define STATE_SECONDARY_KEY 0x03
#define STATE_RELEASED      0x04

#define PRESSED_KEY_BUFFER 10
#define INVALID_PRESSED_KEY 255

typedef struct {
    KeyInfo key_info;
    millisec timestamp;
    uint8_t key_press_counter;
    uint8_t state;
    uint8_t key_index;
} PressedKey;

typedef struct {
    PressedKey keys[PRESSED_KEY_BUFFER];
    uint8_t count;
} PressedKeys;

typedef struct {
    TapDanceState state[MAX_TAPDANCE_KEYS];
    uint8_t active_tapdance_key_count;
    uint8_t last_tapdance_index;
    unsigned long last_tapdance_press_ts;
} TapDance;

class MasterReport {
    public:
        MasterReport(KeyMap* keymap);

        void key_timeout_check(millisec now);
        void handle_master_changed_key(ChangedKeyEvent event, millisec now);
        void handle_slave_changed_key(ChangedKeyEvent event, millisec now);

        inline void handle_key_press(KeyInfo key_info, millisec now);
        inline void handle_key_release(KeyInfo key_info, millisec now);

        void print();
    private:
        void handle_changed_key(ChangedKeyEvent e, KeyInfo key_info, millisec now);
        void print_base_report_to_serial();
        void print_system_report_to_serial();
        void print_multimedia_report_to_serial();
        void print_key_event(const char* prefix, KeyInfo key_info);

        void press(PressedKey *pk, millisec now);
        void release(PressedKey *pk, millisec now);

        void clear();

        inline void send_hid_report();

        inline void run_press_hooks(uint8_t event_key_index, millisec now);
        inline void run_press_hook(uint8_t key_index, millisec now);
        inline void run_release_hooks(uint8_t event_key_index, millisec now);
        inline bool run_release_hook(
            uint8_t key_index, uint8_t event_key_index, millisec now
        );

        inline void press_normal_key(KeyInfo key_info);
        inline void release_normal_key(KeyInfo key_info);

        inline void press_one_shot_modifier_key(PressedKey *pk);
        inline void release_one_shot_modifier_key(PressedKey *pk);

        inline void press_layer_key(uint8_t layer);
        inline void release_layer_key(uint8_t layer);

        inline void press_toggle_layer_key(uint8_t layer);

        inline void press_system_key(KeyInfo key_info);
        inline void release_system_key(KeyInfo key_info);

        inline void press_multimedia_key(KeyInfo key_info);
        inline void release_multimedia_key(KeyInfo key_info);

        inline void press_dual_key(PressedKey *pk);
        inline void release_dual_key(PressedKey *pk);

        inline void press_dual_layer_key(PressedKey *pk);
        inline void release_dual_layer_key(PressedKey *pk);

        inline void press_key_with_mod(KeyInfo key_info);
        inline void release_key_with_mod(KeyInfo key_info);

        inline void press_layer_toggle_or_hold();
        inline void release_layer_toggle_or_hold(PressedKey *pk);

        inline PressedKey* append_to_pressed_keys(KeyInfo key_info, millisec now);
        inline PressedKey* find_in_pressed_keys(KeyInfo key_info);
        inline void remove_from_pressed_keys(PressedKey *pk);
        inline bool delay_key_press(PressedKey *pk);

        uint8_t one_shot_modifiers;
        BaseHIDReport base_hid_report;
        BaseHIDReport last_base_hid_report;
        SystemHIDReport system_hid_report;
        SystemHIDReport last_system_hid_report;
        MultimediaHIDReport multimedia_hid_report;
        MultimediaHIDReport last_multimedia_hid_report;

        uint8_t held_keys_count;
        // Does not matter, if this overflows. We need only equality check
        // This is mostly needed for timeout handlers to detect if there
        // was any keys pressed between the key we are checking timeout for
        // and `now`
        uint8_t key_press_counter;

        KeyMap* keymap;
        PressedKeys pressed_keys;
};
