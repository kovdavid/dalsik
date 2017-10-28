#include "dalsik.h"
#include "keymap.h"
#include "keyboard.h"
#include "master_report.h"
#include "serialcommand.h"

uint8_t cmd_buffer[CMD_LENGTH] = {0};
uint8_t cmd_buffer_index = 0;

void process_serial_command(Keyboard* keyboard, KeyMap* keymap) {
    while (Serial.available()) {
        uint8_t c = Serial.read();
        cmd_buffer[cmd_buffer_index++] = c;

        if (cmd_buffer_index == CMD_LENGTH) {
            uint8_t res = execute_command(keyboard, keymap);
            if (res) {
                Serial.print("CMD_ERROR ");
                Serial.print(res);
                Serial.print("\n");
            } else {
                Serial.println("CMD_OK");
            }
            memset(&cmd_buffer, 0, sizeof(uint8_t)*CMD_LENGTH);
            cmd_buffer_index = 0;
        }
    }
}

uint8_t execute_command(Keyboard* keyboard, KeyMap* keymap) {
    if (memcmp(cmd_buffer, CMD_PREFIX, sizeof(CMD_PREFIX)) != 0) {
        return 1; // Invalid command
    }

    uint8_t* buffer = &(cmd_buffer[sizeof(CMD_PREFIX)]);

#if DEBUG
    Serial.print("BUFFER|");
    Serial.print(cmd_buffer);
    Serial.print("|");
    Serial.print(buffer);
    Serial.print("|");
    Serial.println(cmd_buffer_index);
#endif

    if (buffer[0] == CMD_GET_KEY) {
        uint8_t layer = buffer[1];
        uint8_t row   = buffer[2];
        uint8_t col   = buffer[3];

        if (layer >= MAX_LAYER_COUNT) return 7; // Invalid layer
        if (row >= ROW_PIN_COUNT) return 8; // Invalid row
        if (col >= BOTH_SIDE_COL_PIN_COUNT) return 9; // Invalid col

        serial_print_key(keymap, layer, row, col);

        return 0;
    } else if (buffer[0] == CMD_GET_KEYMAP) {
        for (uint8_t layer = 0; layer < MAX_LAYER_COUNT; layer++) {
            for (uint8_t row = 0; row < ROW_PIN_COUNT; row++) {
                for (uint8_t col = 0; col < BOTH_SIDE_COL_PIN_COUNT; col++) {
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
        if (col >= BOTH_SIDE_COL_PIN_COUNT) return 9; // Invalid col

        keymap->set_key(layer, row, col, KeyInfo { key_type, key });

        return 0;
    } else if (buffer[0] == CMD_CLEAR_KEYMAP) {
        Serial.println("Clearing keymap");
        keymap->eeprom_clear();
        Serial.println("Keymap cleared");
        return 0;
    } else if (buffer[0] == CMD_NUM_ROWS) {
        Serial.print("ROWS<");
        Serial.print(ROW_PIN_COUNT);
        Serial.print(">\n");
        return 0;
    } else if (buffer[0] == CMD_NUM_COLS) {
        Serial.print("COLS<");
        Serial.print(BOTH_SIDE_COL_PIN_COUNT);
        Serial.print(">\n");
        return 0;
    } else if (buffer[0] == CMD_PING) {
        Serial.println("PONG");
        return 0;
    } else if (buffer[0] == CMD_GET_LAYER) {
        Serial.print("Layer<");
        Serial.print(keymap->get_layer(), HEX);
        Serial.print(">\n");
        return 0;
    }

    return 1;
}

void serial_print_key(KeyMap* keymap, uint8_t layer, uint8_t row, uint8_t col) {
    KeyInfo key_info = keymap->get_key_from_layer(layer, row, col);

    Serial.print("KEY<L");
    Serial.print(layer);
    Serial.print("-R");
    Serial.print(row);
    Serial.print("-C");
    Serial.print(col);
    Serial.print("|");
    Serial.print(key_type_to_string(key_info));
    Serial.print("|");
    Serial.print(key_info.key, HEX);
    Serial.print(">\n");
}
