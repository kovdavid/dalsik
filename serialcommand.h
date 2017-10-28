#ifndef SERIALCOMMAND_H
#define SERIALCOMMAND_H

#include "keymap.h"
#include "keyboard.h"

static const char CMD_PREFIX[] = {'D','A','L','S','I','K','-'};

#define CMD_GET_KEY      0x01
#define CMD_GET_KEYMAP   0x02
#define CMD_SET_KEY      0x03
#define CMD_CLEAR_KEYMAP 0x04
#define CMD_NUM_ROWS     0x05
#define CMD_NUM_COLS     0x06
#define CMD_PING         0x07
#define CMD_GET_LAYER    0x08

// +1 for the command type
// +5 is the max size of arguments (for SET_KEY)
// I can't use '\n' as the indicator of the end of a command, because that is
// the HID code for KC_G, so I made every command fixed-length
#define CMD_LENGTH sizeof(CMD_PREFIX)+1+5

namespace SerialCommand {
    void process_command(Keyboard* keyboard, KeyMap* keymap);
}

#endif
