#ifndef KEYBOARD_h
#define KEYBOARD_h

#define DEBUG 0

#define ROW_PIN_COUNT 4
#define COL_PIN_COUNT 4
#define _ROW_PINS A3, A2, A1, A0
#define _COL_PINS 15, 14, 16, 10

const uint8_t ROW_PINS[ROW_PIN_COUNT] = { _ROW_PINS };
const uint8_t COL_PINS[COL_PIN_COUNT] = { _COL_PINS };

typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
} KeyReport;

void keyboard_init();
void keyboard_loop(unsigned long now_msec);
static uint8_t debounce_input(uint8_t* debounce_state, int input);
static void keyboard_matrix_scan(unsigned long now_msec);
static void keyboard_send_report(KeyReport* keys);

#endif
