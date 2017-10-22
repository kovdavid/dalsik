#include "dalsik.h"
#include "keymap.h"
#include "master_report.h"

#define BUFFER_SIZE 64

static const char PREFIX[] = { 'D','A','L','S','I','K','-' };
static const char CMD_SET_KEY_PREFIX[] = { 'S','E','T','_','K','E','Y' };
static const char CMD_GET_KEY_PREFIX[] = { 'G','E','T','_','K','E','Y' };
static const char CMD_KEYMAP_PREFIX[] = { 'K','E','Y','M','A','P' };
static const char CMD_GET_LAYER_PREFIX[] = { 'G','E','T','_','L','A','Y','E','R' };
static const char CMD_SET_LAYER_PREFIX[] = { 'S','E','T','_','L','A','Y','E','R' };
static const char CMD_CLEAR_KEYMAP[] = { 'C','L','E','A','R','_','E','E','P','R','O','M' };
static const char CMD_PING[] = { 'P','I','N','G' };

char serial_buffer[BUFFER_SIZE] = {0};
uint8_t serial_index = 0;

void process_serial_command(Keyboard* keyboard, KeyMap* keymap) {
    while (Serial.available()) {
        char c = (char) Serial.read();

        if (c == '\n') {
            if (memcmp(serial_buffer, PREFIX, sizeof(PREFIX)) == 0) {
                uint8_t res = execute_command(keyboard, keymap);
                if (res) {
                    Serial.print("CMD_ERROR ");
                    Serial.print(res);
                    Serial.print("\n");
                } else {
                    Serial.println("CMD_OK");
                }
            } else {
                Serial.println("CMD_PARSE_ERROR");
            }
            clear_serial_buffer();
        } else {
            if (serial_index > BUFFER_SIZE) {
                clear_serial_buffer();
                return;
            } else {
                serial_buffer[serial_index++] = c;
            }
        }
    }
}

uint8_t execute_command(Keyboard* keyboard, KeyMap* keymap) {
    char* buffer = &(serial_buffer[sizeof(PREFIX)]);

#if DEBUG
    Serial.print("BUFFER|");
    Serial.print(serial_buffer);
    Serial.print("|");
    Serial.print(buffer);
    Serial.print("|\n");
    Serial.println(serial_index);
#endif

    if (memcmp(buffer, CMD_SET_KEY_PREFIX, sizeof(CMD_SET_KEY_PREFIX)) == 0) {
        if (sizeof(PREFIX) + sizeof(CMD_SET_KEY_PREFIX) + 5 > serial_index) {
            return 2; // Incomplete data
        }

        buffer = &(serial_buffer[sizeof(PREFIX) + sizeof(CMD_SET_KEY_PREFIX)]);
        uint8_t layer = buffer[0x00];
        uint8_t row = buffer[0x01];
        uint8_t col = buffer[0x02];
        uint8_t key_type = buffer[0x03];
        uint8_t key = buffer[0x04];

#if DEBUG
        Serial.print("SET_KEY<");
        for (int i = 0; i < 6; i++) {
            Serial.print(buffer[i], HEX);
        }
        Serial.print(">\n");
#endif

        if (layer >= MAX_LAYER_COUNT) return 3; // Invalid layer
        if (row >= ROW_PIN_COUNT) return 4; // Invalid row
        if (col >= BOTH_SIDE_COL_PIN_COUNT) return 5; // Invalid col

        KeyInfo key_info = { key_type, key };
        keymap->set_key(layer, row, col, key_info);

        return 0;
    } else if (memcmp(buffer, CMD_GET_KEY_PREFIX, sizeof(CMD_GET_KEY_PREFIX)) == 0) {
        if (sizeof(PREFIX) + sizeof(CMD_GET_KEY_PREFIX) + 3 > serial_index) {
            return 6; // Incomplete data
        }

        buffer = &(serial_buffer[sizeof(PREFIX) + sizeof(CMD_GET_KEY_PREFIX)]);
        uint8_t layer = buffer[0x00];
        uint8_t row   = buffer[0x01];
        uint8_t col   = buffer[0x02];

        if (layer >= MAX_LAYER_COUNT) return 7; // Invalid layer
        if (row >= ROW_PIN_COUNT) return 8; // Invalid row
        if (col >= BOTH_SIDE_COL_PIN_COUNT) return 9; // Invalid col

        serial_print_key(keymap, layer, row, col);

        return 0;
    } else if (memcmp(buffer, CMD_KEYMAP_PREFIX, sizeof(CMD_KEYMAP_PREFIX)) == 0) {
        for (uint8_t layer = 0; layer < MAX_LAYER_COUNT; layer++) {
            for (uint8_t row = 0; row < ROW_PIN_COUNT; row++) {
                for (uint8_t col = 0; col < BOTH_SIDE_COL_PIN_COUNT; col++) {
                    serial_print_key(keymap, layer, row, col);
                }
            }
        }
        return 0;
    } else if (memcmp(buffer, CMD_GET_LAYER_PREFIX, sizeof(CMD_GET_LAYER_PREFIX)) == 0) {
        Serial.print("Layer<");
        Serial.print(keymap->get_layer(), HEX);
        Serial.print(">\n");
        return 0;
    } else if (memcmp(buffer, CMD_SET_LAYER_PREFIX, sizeof(CMD_SET_LAYER_PREFIX)) == 0) {
        if (sizeof(PREFIX) + sizeof(CMD_GET_KEY_PREFIX) + 1 > serial_index) {
            return 6; // Incomplete data
        }

        buffer = &(serial_buffer[sizeof(PREFIX) + sizeof(CMD_GET_KEY_PREFIX)]);
        uint8_t layer = buffer[0x00];

        if (layer >= MAX_LAYER_COUNT) {
            return 7; // Invalid layer
        }

        keymap->set_layer(layer);
        return 0;
    } else if (memcmp(buffer, CMD_CLEAR_KEYMAP, sizeof(CMD_CLEAR_KEYMAP)) == 0) {
        Serial.println("Clearing keymap");
        keymap->eeprom_clear();
        Serial.println("Keymap cleared");
        return 0;
    } else if (memcmp(buffer, CMD_PING, sizeof(CMD_PING)) == 0) {
        Serial.println("PONG");
        return 0;
    }

    return 1;
}

void clear_serial_buffer() {
    memset(&serial_buffer, 0, sizeof(char)*BUFFER_SIZE);
    serial_index = 0;
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
