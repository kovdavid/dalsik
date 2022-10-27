#pragma once

#include "matrix.h"
#include "dalsik.h"
#include "dalsik_hid.h"
#include "dalsik_global.h"

#ifdef LED_PIN
const uint32_t LED_LAYER_COLORS[MAX_LAYER_COUNT] = {
    0x00000000, // Layer 0 - default (LED off)
    0x00FF0000,
    0x0000FF00,
    0x000000FF,
    0x00FFFF00,
    0x00FF00FF,
    0x00000000,
    0x00000000,
};
#endif

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

class Keyboard {
    private:
        // State
        uint8_t layer_index;
        uint8_t toggled_layer_index;
        uint8_t layer_history[LAYER_HISTORY_CAPACITY];

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

        PressedKeys pressed_keys;


        // Functions

        void set_layer(uint8_t layer);
        void remove_layer(uint8_t layer);
        void toggle_layer(uint8_t layer);
        void clear();

        KeyInfo get_non_transparent_key(KeyCoords c);
        KeyInfo get_key(KeyCoords c);
        void reload_keys_on_new_layer(uint8_t key_index);

        inline void handle_key_press(KeyInfo key_info, millisec now);
        inline void handle_key_release(KeyInfo key_info, millisec now);

        void press(PressedKey *pk, millisec now);
        void release(PressedKey *pk, millisec now);
        inline void run_press_hooks(uint8_t event_key_index, millisec now);
        inline void run_press_hook(uint8_t key_index, millisec now);
        inline void run_release_hooks(uint8_t event_key_index, millisec now);
        inline bool run_release_hook(uint8_t key_index, uint8_t event_key_index, millisec now);

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

        inline void press_layer_toggle_or_hold(PressedKey *pk);
        inline void release_layer_toggle_or_hold(PressedKey *pk);

        inline void send_hid_report();

        inline PressedKey* add_to_pressed_keys(KeyInfo key_info, millisec now);
        inline PressedKey* find_in_pressed_keys(KeyInfo key_info);
        inline void remove_from_pressed_keys(PressedKey *pk);

        void print_base_report_to_serial();
        void print_system_report_to_serial();
        void print_multimedia_report_to_serial();

    public:
        Keyboard(void);

        void handle_changed_key(ChangedKeyEvent e, millisec now);
        void key_timeout_check(millisec now);

        void print_internal_state();
};
