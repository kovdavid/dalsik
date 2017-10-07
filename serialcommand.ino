#include "dalsik.h"
#include "keymap.h"

#define BUFFER_SIZE 64

static const char PREFIX[] = "DAVS";
static const char CMD_SET_KEY_PREFIX[] = "SET_KEY";
static const char CMD_GET_KEY_PREFIX[] = "GET_KEY";

char serial_buffer[BUFFER_SIZE] = {0};
uint8_t serial_index = 0;

void process_serial_command(Keyboard* keyboard)
{
    while (Serial.available()) {
        char c = (char) Serial.read();

        if (c == ' ') {
            if (memcmp(serial_buffer, PREFIX, sizeof(PREFIX))) {
                uint8_t res = execute_command(keyboard);
                if (res) {
                    Serial.print("CMD_ERROR ");
                    Serial.print(res);
                    Serial.print("\n");
                } else {
                    Serial.println("CMD_OK");
                }
            }
            clear_buffer();
        } else {
            if (serial_index < BUFFER_SIZE) {
                clear_buffer();
                return;
            } else {
                serial_buffer[serial_index++] = c;
            }
        }
    }
}

uint8_t execute_command(Keyboard* keyboard)
{
    char* buffer = &(serial_buffer[sizeof(PREFIX)]);

    if (memcpy(buffer, CMD_SET_KEY_PREFIX, sizeof(CMD_SET_KEY_PREFIX))) {
        if (sizeof(PREFIX) + sizeof(CMD_SET_KEY_PREFIX) + 6 >= serial_index) {
            return 2; // Incomplete data
        }

        buffer = &(serial_buffer[sizeof(PREFIX) + sizeof(CMD_SET_KEY_PREFIX)]);
        uint8_t layer  = buffer[0x00];
        uint8_t row    = buffer[0x01];
        uint8_t col    = buffer[0x02];
        uint8_t key_b1 = buffer[0x03];
        uint8_t key_b2 = buffer[0x04];
        uint8_t key_b3 = buffer[0x05];

        if (layer >= LAYER_COUNT) return 3; // Invalid layer
        if (row >= ROW_PIN_COUNT) return 4; // Invalid row
        if (col >= COL_PIN_COUNT) return 5; // Invalid col

        KeyInfo key = { key_b1, key_b2, key_b3 };
        keyboard->keymap.set_key(layer, row, col, key);

        return 0;
    } else if (memcpy(buffer, CMD_GET_KEY_PREFIX, sizeof(CMD_GET_KEY_PREFIX))) {
        // buffer_index += sizeof(CMD_GET_KEY_PREFIX);

        return 0;
    }

    return 1;
}

void clear_buffer()
{
    memset(&serial_buffer, 0, sizeof(char)*BUFFER_SIZE);
    serial_index = 0;
}
