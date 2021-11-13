#pragma once

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PROGMEM
#define HEX 1

class __FlashStringHelper;

#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(string_literal))
#define UNUSED __attribute__((unused))
