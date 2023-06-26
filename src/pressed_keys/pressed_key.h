#pragma once

#include "dalsik.h"
#include "key.h"

#define STATE_NOT_PROCESSED    0x00
#define STATE_PENDING          0x01
#define STATE_ACTIVE_CODE      0x02
#define STATE_ACTIVE_MODIFIERS 0x03
#define STATE_ACTIVE_LAYER     0x04
#define STATE_RELEASED         0x05

typedef struct {
    Key key;
    millisec timestamp;
    uint8_t key_press_counter;
    uint8_t state : 4;
    uint8_t key_index : 4;
} PressedKey;
