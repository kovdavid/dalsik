#ifndef DALSIK_H
#define DALSIK_H

#define DEBUG 0

#define IS_MASTER 1

#define MASTER_SIDE_LEFT 0x00
#define MASTER_SIDE_RIGHT 0x01
// Which side has the USB cable plugged?
#define MASTER_SIDE MASTER_SIDE_LEFT

// Use I2C or serial?
#define USE_I2C 0
#define I2C_MASTER_ADDRESS 0x01
#define I2C_SLAVE_ADDRESS  0x02

#define MAX_LAYER_COUNT 8
#define ROW_PIN_COUNT 4
#define ONE_SIDE_COL_PIN_COUNT 4
#define BOTH_SIDE_COL_PIN_COUNT 2*ONE_SIDE_COL_PIN_COUNT
#define _ROW_PINS 6, 7, 8, 9
#define _COL_PINS A1, A0, 15, 14
// FULL _COL_PINS A1, A0, 15, 14, 16, 10

// Turn off the keyboard with this PIN
#define ON_OFF_PIN 4

const uint8_t ROW_PINS[ROW_PIN_COUNT] = { _ROW_PINS };
const uint8_t COL_PINS[ONE_SIDE_COL_PIN_COUNT] = { _COL_PINS };

#endif
