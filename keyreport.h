#ifndef KEYREPORT_h
#define KEYREPORT_h

#include "keymap.h"

typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
} HIDKeyboardReport;

#define DUAL_MODE_NOT_PRESSED   0x00
#define DUAL_MODE_PENDING       0x01
#define DUAL_MODE_TAP_KEY       0x02
#define DUAL_MODE_HOLD_MODIFIER 0x03

typedef struct {
    uint8_t mode;
    KeyDual key;
} DualKeyState;

class KeyReport
{
    private:
        void print_to_serial();
    public:
        KeyReport(void);

        void clear();
        void send_report();
        void check_special_keys(unsigned long now_msec);
        void dual_state_press_hook(KeyInfo key);

        void press(unsigned long now_msec, uint8_t row, uint8_t col);
        void press_normal_key(KeyInfo key);
        void press_layer_key(KeyInfo key);
        void press_dual_key(KeyInfo key);

        void release(unsigned long now_msec, uint8_t row, uint8_t col);
        void release_normal_key(KeyInfo key);
        void release_layer_key(KeyInfo key);
        void release_dual_key(KeyInfo key);

        HIDKeyboardReport hid_report;
        KeyMap keymap;
        DualKeyState dual_key_state;
        uint8_t keys_pressed;
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
