#ifndef MASTER_REPORT_H
#define MASTER_REPORT_H

#include "keyboard.h"
#include "keymap.h"

#define HID_KEYS_COUNT 6

typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[HID_KEYS_COUNT];
} HIDKeyboardReport;

#define DUAL_MODE_NOT_PRESSED   0x00
#define DUAL_MODE_PENDING       0x01
#define DUAL_MODE_TAP_KEY       0x02
#define DUAL_MODE_HOLD_MODIFIER 0x03

typedef struct {
    uint8_t mode;
    KeyInfo key_info;
} DualKeyState;

class MasterReport {
    private:
        void print_to_serial();
    public:
        MasterReport(KeyMap* keymap);

        void handle_changed_key(ChangedKeyCoords coords);

        void clear();
        void send_report();
        void check_special_keys();
        void press_hook_for_dual_keys();

        void press(KeyInfo key_info);
        void press_normal_key(KeyInfo key_info);
        void press_layer_key(KeyInfo key_info);
        void press_toggle_layer_key(KeyInfo key_info);
        void press_dual_key(KeyInfo key_info);

        void release(KeyInfo key_info);
        void release_normal_key(KeyInfo key_info);
        void release_layer_key(KeyInfo key_info);
        void release_toggle_layer_key(KeyInfo key_info);
        void release_dual_key(KeyInfo key_info);

        HIDKeyboardReport hid_report;
        KeyMap* keymap;
        DualKeyState dual_key_state;
        uint8_t num_keys_pressed;
};

#endif

// bit number
// 76543210
// DEADBEEF

// bit 0: left control
// bit 1: left shift
// bit 2: left alt
// bit 3: left GUI (Win/Apple/Meta key)
// bit 4: right control
// bit 5: right shift
// bit 6: right alt
// bit 7: right GUI
//
// E0 Left control
// E1 Left shift
// E2 Left alt
// E3 Left GUI
// E4 Right control
// E5 Right shift
// E6 Right alt
// E7 Right GUI
