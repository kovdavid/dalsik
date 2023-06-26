#pragma once

#include "dalsik.h"

#define KEYBOARD_SIDE_ADDRESS 0

#define KEYBOARD_SIDE_LEFT  'L'
#define KEYBOARD_SIDE_RIGHT 'R'

namespace EEPROM {
    char get_keyboard_side();
    void set_keyboard_side(char side);
}
