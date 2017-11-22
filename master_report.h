#ifndef MASTER_REPORT_H
#define MASTER_REPORT_H

#include "matrix.h"
#include "keymap.h"
#include "dalsik_hid_desc.h"

#define DUAL_MODE_NOT_PRESSED   0x00
#define DUAL_MODE_PENDING       0x01
#define DUAL_MODE_HOLD_MODIFIER 0x02

typedef struct {
    uint8_t mode;
    KeyInfo key_info;
} DualKeyState;

#define HOLD_OR_TOGGLE_NOT_PRESSED 0x00
#define HOLD_OR_TOGGLE_PENDING     0x01
#define HOLD_OR_TOGGLE_HOLD_LAYER  0x02

typedef struct {
    uint8_t mode;
    KeyInfo key_info;
} LayerHoldOrToggleState;

class MasterReport {
    private:
        void print_to_serial();
    public:
        MasterReport(KeyMap* keymap);

        void clear();
        void send_report();
        inline void press_hook_for_dual_keys();
        inline void press_hook_for_layer_hold_or_toggle();

        void handle_master_changed_key(ChangedKeyCoords coords);
        void handle_slave_changed_key(ChangedKeyCoords coords);
        void handle_changed_key(ChangedKeyCoords coords);
        void press(KeyInfo key_info);
        void release(KeyInfo key_info);

        inline void press_normal_key(KeyInfo key_info);
        inline void release_normal_key(KeyInfo key_info);

        inline void press_layer_key(KeyInfo key_info);
        inline void release_layer_key(KeyInfo key_info);

        inline void press_toggle_layer_key(KeyInfo key_info);
        inline void release_toggle_layer_key(KeyInfo key_info);

        inline void press_dual_key(KeyInfo key_info);
        inline void release_dual_key(KeyInfo key_info);

        inline void press_key_with_mod(KeyInfo key_info);
        inline void release_key_with_mod(KeyInfo key_info);

        inline void press_layer_hold_or_toggle(KeyInfo key_info);
        inline void release_layer_hold_or_toggle(KeyInfo key_info);

        BaseHIDReport base_hid_report;
        SystemHIDReport system_hid_report;
        MultimediaHIDReport multimedia_hid_report;

        uint8_t base_hid_report_changed;
        uint8_t system_hid_report_changed;
        uint8_t multimedia_hid_report_changed;

        KeyMap* keymap;
        DualKeyState dual_key_state;
        LayerHoldOrToggleState hold_or_toggle_state;
        uint8_t num_keys_pressed;
};

#endif
