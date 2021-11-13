#include <Arduino.h>
#include "dalsik.h"
#include "dalsik_eeprom.h"
#include "serial_command.h"

static const char CMD_PREFIX[] = {'D','A','L','S','I','K','-'};

#define CMD_GET_KEY             0x01
#define CMD_GET_KEYMAP          0x02
#define CMD_SET_KEY             0x03
#define CMD_CLEAR_KEYMAP        0x04
#define CMD_NUM_ROWS            0x05
#define CMD_NUM_COLS            0x06
#define CMD_PING                0x07
#define CMD_CLEAR_TAPDANCE      0x09
#define CMD_SET_TAPDANCE_KEY    0x0A
#define CMD_GET_TAPDANCE_KEY    0x0B
#define CMD_GET_TAPDANCE_KEYMAP 0x0C
#define CMD_CLEAR_EEPROM        0x0D
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
        for (uint8_t layer = 0; layer < MAX_LAYER_COUNT; layer++) {
            for (uint8_t row = 0; row < ROW_PIN_COUNT; row++) {
                for (uint8_t col = 0; col < 2*ONE_SIDE_COL_PIN_COUNT; col++) {
                    serial_print_key(layer, KeyCoords { row, col });
                }
            }
        }
        return 0;
    } else if (buffer[0] == CMD_SET_KEY) {
        uint8_t layer    = buffer[1];
        uint8_t row      = buffer[2];
        uint8_t col      = buffer[3];
        uint8_t key_type = buffer[4];
        uint8_t key      = buffer[5];

        if (layer >= MAX_LAYER_COUNT) return 7; // Invalid layer
        if (row >= ROW_PIN_COUNT) return 8; // Invalid row
        if (col >= 2*ONE_SIDE_COL_PIN_COUNT) return 9; // Invalid col

        KeyCoords c = { row, col };
        KeyInfo ki = KeyInfo(key_type, key, c);
        EEPROM::set_key(layer, ki);

        Serial.print(F("SET_KEY<LAYER:"));
        Serial.print(layer);
        Serial.print(F("|ROW:"));
        Serial.print(row);
        Serial.print(F("|COL:"));
        Serial.print(col);
        Serial.print(F("|TYPE:"));
        Serial.print(key_type);
        Serial.print(F("|KEY:"));
        Serial.print(key, HEX);
        Serial.print(F(">\n"));

        return 0;
    } else if (buffer[0] == CMD_CLEAR_KEYMAP) {
        Serial.println(F("Clearing keymap"));
        EEPROM::clear_keymap();
        Serial.println(F("Keymap cleared"));
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
    } else if (buffer[0] == CMD_CLEAR_TAPDANCE) {
        Serial.println(F("Clearing tapdance"));
        EEPROM::clear_tapdance();
        Serial.println(F("Tapdance cleared"));
        return 0;
    } else if (buffer[0] == CMD_SET_TAPDANCE_KEY) {
        uint8_t index    = buffer[1];
        uint8_t tap      = buffer[2];
        uint8_t key_type = buffer[3];
        uint8_t key      = buffer[4];

        if (index >= MAX_TAPDANCE_KEYS) return 7;
        if (tap > MAX_TAPDANCE_TAPS) return 8;

        KeyInfo ki = KeyInfo(key_type, key);
        EEPROM::set_tapdance_key(index, tap, ki);

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
    } else if (buffer[0] == CMD_CLEAR_EEPROM) {
        Serial.println(F("Clearing eeprom"));
        EEPROM::clear_all();
        Serial.println(F("EEPROM cleared"));
        return 0;
    } else if (buffer[0] == CMD_GET_FULL_KEYMAP) {
        for (uint8_t layer = 0; layer < MAX_LAYER_COUNT; layer++) {
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
    KeyInfo key_info = EEPROM::get_key(layer, coords);

    Serial.print(F("KEY<L"));
    Serial.print(layer);
    Serial.print(F("-R"));
    Serial.print(coords.row);
    Serial.print(F("-C"));
    Serial.print(coords.col);
    Serial.print(F("|"));
    Serial.print(key_info.type, HEX);
    Serial.print(F("|"));
    Serial.print(key_info.key, HEX);
    Serial.print(F(">\n"));
}

void serial_print_tapdance_key(uint8_t index, uint8_t tap) {
    KeyInfo key_info = EEPROM::get_tapdance_key(index, tap);

    Serial.print(F("TAPDANCE<I"));
    Serial.print(index);
    Serial.print(F("-T"));
    Serial.print(tap);
    Serial.print(F("|"));
    Serial.print(key_info.type, HEX);
    Serial.print(F("|"));
    Serial.print(key_info.key, HEX);
    Serial.print(F(">\n"));
}
