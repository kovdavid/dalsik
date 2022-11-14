#include <Arduino.h>
#include "dalsik.h"
#include "dalsik_eeprom.h"
#include "serial_command.h"
#include "keymap.h"

static const char CMD_PREFIX[] = {'D','A','L','S','I','K','-'};

#define CMD_GET_KEY             0x01
#define CMD_GET_KEYMAP          0x02
#define CMD_NUM_ROWS            0x05
#define CMD_NUM_COLS            0x06
#define CMD_PING                0x07
#define CMD_GET_TAPDANCE_KEY    0x0B
#define CMD_GET_TAPDANCE_KEYMAP 0x0C
#define CMD_GET_FULL_KEYMAP     0x0E
#define CMD_GET_KEYBOARD_SIDE   0x0F
#define CMD_SET_KEYBOARD_SIDE   0x10

// +1 for the command type
// +5 is the max size of arguments (for SET_KEY)
// I can't use '\n' as the indicator of the end of a command, because that is
// the HID code for KC_G, so I made every command fixed-length
#define CMD_LENGTH sizeof(CMD_PREFIX)+1+5

static uint8_t execute_command();
static void serial_print_key(uint8_t layer, KeyCoords coords);
static void serial_print_tapdance_key(uint8_t index, uint8_t tap);

char cmd_buffer[CMD_LENGTH] = {0};
uint8_t cmd_buffer_index = 0;

void SerialCommand::process_command() {
    char c = Serial.read();
    cmd_buffer[cmd_buffer_index++] = c;

    if (cmd_buffer_index == CMD_LENGTH) {
        uint8_t res = execute_command();
        if (res) {
            Serial.print(F("CMD_ERROR "));
            Serial.print(res);
            Serial.print(F("\n"));
        } else {
            Serial.println(F("CMD_OK"));
        }
        memset(&cmd_buffer, 0, sizeof(char)*CMD_LENGTH);
        cmd_buffer_index = 0;
    }
}

uint8_t execute_command() {
    if (memcmp(cmd_buffer, CMD_PREFIX, sizeof(CMD_PREFIX)) != 0) {
        return 1; // Invalid command
    }

    char* buffer = &(cmd_buffer[sizeof(CMD_PREFIX)]);

    if (buffer[0] == CMD_GET_KEY) {
        uint8_t layer = buffer[1];
        uint8_t row   = buffer[2];
        uint8_t col   = buffer[3];

        if (layer >= MAX_LAYER_COUNT) return 7;       // Invalid layer
        if (row >= ROW_PIN_COUNT) return 8;           // Invalid row
        if (col >= 2*ONE_SIDE_COL_PIN_COUNT) return 9; // Invalid col

        serial_print_key(layer, KeyCoords { row, col });

        return 0;
    } else if (buffer[0] == CMD_GET_KEYMAP) {
        for (uint8_t layer = 0; layer < KeyMap::get_layer_count(); layer++) {
            for (uint8_t row = 0; row < ROW_PIN_COUNT; row++) {
                for (uint8_t col = 0; col < 2*ONE_SIDE_COL_PIN_COUNT; col++) {
                    serial_print_key(layer, KeyCoords { row, col });
                }
            }
        }
        return 0;
    } else if (buffer[0] == CMD_NUM_ROWS) {
        Serial.print(F("ROWS<"));
        Serial.print(ROW_PIN_COUNT);
        Serial.print(F(">\n"));
        return 0;
    } else if (buffer[0] == CMD_NUM_COLS) {
        Serial.print(F("COLS<"));
        Serial.print(2*ONE_SIDE_COL_PIN_COUNT);
        Serial.print(F(">\n"));
        return 0;
    } else if (buffer[0] == CMD_PING) {
        Serial.println(F("PONG"));
        return 0;
    } else if (buffer[0] == CMD_GET_TAPDANCE_KEY) {
        uint8_t index = buffer[1];
        uint8_t tap   = buffer[2];

        if (index >= MAX_TAPDANCE_KEYS) return 7;
        if (tap > MAX_TAPDANCE_TAPS) return 8;

        serial_print_tapdance_key(index, tap);

        return 0;
    } else if (buffer[0] == CMD_GET_TAPDANCE_KEYMAP) {
        for (uint8_t index = 0; index < MAX_TAPDANCE_KEYS; index++) {
            for (uint8_t tap = 1; tap <= MAX_TAPDANCE_TAPS; tap++) {
                serial_print_tapdance_key(index, tap);
            }
        }

        return 0;
    } else if (buffer[0] == CMD_GET_FULL_KEYMAP) {
        for (uint8_t layer = 0; layer < KeyMap::get_layer_count(); layer++) {
            for (uint8_t row = 0; row < ROW_PIN_COUNT; row++) {
                for (uint8_t col = 0; col < 2*ONE_SIDE_COL_PIN_COUNT; col++) {
                    serial_print_key(layer, KeyCoords { row, col });
                }
            }
        }
        for (uint8_t index = 0; index < MAX_TAPDANCE_KEYS; index++) {
            for (uint8_t tap = 1; tap <= MAX_TAPDANCE_TAPS; tap++) {
                serial_print_tapdance_key(index, tap);
            }
        }
        return 0;
    } else if (buffer[0] == CMD_GET_KEYBOARD_SIDE) {
        uint8_t keyboard_side = EEPROM::get_keyboard_side();
        if (keyboard_side == KEYBOARD_SIDE_LEFT) {
            Serial.println(F("Keyboard Side <LEFT>"));
        } else {
            Serial.println(F("Keyboard Side <RIGHT>"));
        }
        return 0;
    } else if (buffer[0] == CMD_SET_KEYBOARD_SIDE) {
        uint8_t side = buffer[1];
        if (side == 'L') {
            EEPROM::set_keyboard_side(KEYBOARD_SIDE_LEFT);
            return 0;
        }
        if (side == 'R') {
            EEPROM::set_keyboard_side(KEYBOARD_SIDE_RIGHT);
            return 0;
        }
        return 9;
    }

    return 1;
}

void serial_print_key(uint8_t layer, KeyCoords coords) {
    KeyInfo key_info = KeyMap::get_key(layer, coords);

    Serial.print(F("KEY<L"));
    Serial.print(layer);
    Serial.print(F("-R"));
    Serial.print(coords.row);
    Serial.print(F("-C"));
    Serial.print(coords.col);
    Serial.print(F("|T"));
    Serial.print(key_info.type, HEX);
    Serial.print(F("|L"));
    Serial.print(key_info.layer, HEX);
    Serial.print(F("|M"));
    Serial.print(key_info.mod, HEX);
    Serial.print(F("|K"));
    Serial.print(key_info.key, HEX);
    Serial.print(F(">\n"));
}

void serial_print_tapdance_key(uint8_t index, uint8_t tap) {
}
