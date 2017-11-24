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
#define ONE_SIDE_COL_PIN_COUNT 6
#define BOTH_SIDE_COL_PIN_COUNT 2*ONE_SIDE_COL_PIN_COUNT

// Turn off the keyboard with this PIN
#define ON_OFF_PIN 4

const uint8_t ROW_PINS[ROW_PIN_COUNT] = { 0xD7, 0xE6, 0xB4, 0xB5 };
const uint8_t COL_PINS[ONE_SIDE_COL_PIN_COUNT] = { 0xF6, 0xF7, 0xB1, 0xB3, 0xB2, 0xB6 };

#endif
