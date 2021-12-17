#include "dalsik.h"
#include "keymap.h"
#include "serial_command.h"
#include "serial_command.h"

static uint8_t execute_command(KeyMap* keymap);
static void serial_print_key(KeyMap* keymap, uint8_t layer, uint8_t row, uint8_t col);
static void serial_print_tapdance_key(KeyMap* keymap, uint8_t index, uint8_t tap);

char cmd_buffer[CMD_LENGTH] = {0};
uint8_t cmd_buffer_index = 0;

void SerialCommand::process_command(KeyMap* keymap) {
    char c = Serial.read();
    cmd_buffer[cmd_buffer_index++] = c;

    if (cmd_buffer_index == CMD_LENGTH) {
        uint8_t res = execute_command(keymap);
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

static uint8_t execute_command(KeyMap* keymap) {
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

        serial_print_key(keymap, layer, row, col);

        return 0;
    } else if (buffer[0] == CMD_GET_KEYMAP) {
        for (uint8_t layer = 0; layer < MAX_LAYER_COUNT; layer++) {
            for (uint8_t row = 0; row < ROW_PIN_COUNT; row++) {
                for (uint8_t col = 0; col < 2*ONE_SIDE_COL_PIN_COUNT; col++) {
                    serial_print_key(keymap, layer, row, col);
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

        KeyInfo ki = KeyMap::init_key_info(key_type, key, row, col);
        keymap->set_key(layer, ki);

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
        keymap->eeprom_clear_keymap();
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
    } else if (buffer[0] == CMD_GET_LAYER) {
        Serial.print(F("Layer<"));
        Serial.print(keymap->get_layer(), HEX);
        Serial.print(F(">\n"));
        return 0;
    } else if (buffer[0] == CMD_CLEAR_TAPDANCE) {
        Serial.println(F("Clearing tapdance"));
        keymap->eeprom_clear_tapdance();
        Serial.println(F("Tapdance cleared"));
        return 0;
    } else if (buffer[0] == CMD_SET_TAPDANCE_KEY) {
        uint8_t index    = buffer[1];
        uint8_t tap      = buffer[2];
        uint8_t key_type = buffer[3];
        uint8_t key      = buffer[4];

        if (index >= MAX_TAPDANCE_KEYS) return 7;
        if (tap > MAX_TAPDANCE_TAPS) return 8;

        KeyInfo ki = KeyMap::init_key_info_without_coords(key_type, key);
        keymap->set_tapdance_key(index, tap, ki);

        return 0;
    } else if (buffer[0] == CMD_GET_TAPDANCE_KEY) {
        uint8_t index = buffer[1];
        uint8_t tap   = buffer[2];

        if (index >= MAX_TAPDANCE_KEYS) return 7;
        if (tap > MAX_TAPDANCE_TAPS) return 8;

        serial_print_tapdance_key(keymap, index, tap);

        return 0;
    } else if (buffer[0] == CMD_GET_TAPDANCE_KEYMAP) {
        for (uint8_t index = 0; index < MAX_TAPDANCE_KEYS; index++) {
            for (uint8_t tap = 1; tap <= MAX_TAPDANCE_TAPS; tap++) {
                serial_print_tapdance_key(keymap, index, tap);
            }
        }

        return 0;
    } else if (buffer[0] == CMD_CLEAR_EEPROM) {
        Serial.println(F("Clearing eeprom"));
        keymap->eeprom_clear_all();
        Serial.println(F("EEPROM cleared"));
        return 0;
    } else if (buffer[0] == CMD_GET_FULL_KEYMAP) {
        for (uint8_t layer = 0; layer < MAX_LAYER_COUNT; layer++) {
            for (uint8_t row = 0; row < ROW_PIN_COUNT; row++) {
                for (uint8_t col = 0; col < 2*ONE_SIDE_COL_PIN_COUNT; col++) {
                    serial_print_key(keymap, layer, row, col);
                }
            }
        }
        for (uint8_t index = 0; index < MAX_TAPDANCE_KEYS; index++) {
            for (uint8_t tap = 1; tap <= MAX_TAPDANCE_TAPS; tap++) {
                serial_print_tapdance_key(keymap, index, tap);
            }
        }
        return 0;
    } else if (buffer[0] == CMD_GET_KEYBOARD_SIDE) {
        uint8_t keyboard_side = keymap->get_keyboard_side();
        if (keyboard_side == KEYBOARD_SIDE_LEFT) {
            Serial.println(F("Keyboard Side <LEFT>"));
        } else {
            Serial.println(F("Keyboard Side <RIGHT>"));
        }
        return 0;
    } else if (buffer[0] == CMD_SET_KEYBOARD_SIDE) {
        uint8_t side = buffer[1];
        if (side == 'L') {
            keymap->update_keyboard_side(KEYBOARD_SIDE_LEFT);
            return 0;
        }
        if (side == 'R') {
            keymap->update_keyboard_side(KEYBOARD_SIDE_RIGHT);
            return 0;
        }
        return 9;
    }

    return 1;
}

static void serial_print_key(KeyMap* keymap, uint8_t layer, uint8_t row, uint8_t col) {
    KeyInfo key_info = keymap->get_key_from_layer(layer, row, col);

    Serial.print(F("KEY<L"));
    Serial.print(layer);
    Serial.print(F("-R"));
    Serial.print(row);
    Serial.print(F("-C"));
    Serial.print(col);
    Serial.print(F("|"));
    Serial.print(KeyMap::key_type_to_string(key_info));
    Serial.print(F("|"));
    Serial.print(key_info.key, HEX);
    Serial.print(F(">\n"));
}

static void serial_print_tapdance_key(KeyMap* keymap, uint8_t index, uint8_t tap) {
    KeyInfo key_info = keymap->get_tapdance_key(index, tap);

    Serial.print(F("TAPDANCE<I"));
    Serial.print(index);
    Serial.print(F("-T"));
    Serial.print(tap);
    Serial.print(F("|"));
    Serial.print(KeyMap::key_type_to_string(key_info));
    Serial.print(F("|"));
    Serial.print(key_info.key, HEX);
    Serial.print(F(">\n"));
}
