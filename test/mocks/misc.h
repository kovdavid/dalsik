#pragma once

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_KEYBOARD_FRIENDS \
   friend PressedKeys get_keyboard_pressed_keys(Keyboard);

#define PROGMEM
#define HEX 1

class __FlashStringHelper;

#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(string_literal))
#define UNUSED __attribute__((unused))
