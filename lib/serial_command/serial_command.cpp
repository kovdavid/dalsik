#include <Arduino.h>
#include "serial_command.h"

static const char CMD_PREFIX[] = {'D','A','L','S','I','K','-'};

#define CMD_GET_CHECKSUM 0x01

// +1 for the command type
// I can't use '\n' as the indicator of the end of a command, because that is
// the HID code for KC_G, so I made every command fixed-length
#define CMD_LENGTH sizeof(CMD_PREFIX)+1

static uint8_t execute_command();

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

    char command_type = cmd_buffer[sizeof(CMD_PREFIX)];

    if (command_type == CMD_GET_CHECKSUM) {
        Serial.println(DALSIK_CHECKSUM);
        return 0;
    }

    return 1;
}
