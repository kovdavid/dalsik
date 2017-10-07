#ifndef KEYBOARD_h
#define KEYBOARD_h

#include "keyreport.h"
#include "keymap.h"
#include "dalsik.h"

class Keyboard
{
    private:
        void send_report(KeyReport* report);
        void matrix_scan(unsigned long now_msec);
        void clear_report(void);
        uint8_t debounce_input(uint8_t row, uint8_t col, uint8_t input);

        uint8_t keystate[ROW_PIN_COUNT][COL_PIN_COUNT];
        uint8_t debounce[ROW_PIN_COUNT][COL_PIN_COUNT];
    public:
        Keyboard(void);
        void loop(unsigned long now_msec);

        uint8_t keys_pressed;
        KeyReport keyreport;
        KeyMap keymap;
};

#endif
