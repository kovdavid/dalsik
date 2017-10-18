#ifndef DALSIK_H
#define DALSIK_H

#define DEBUG 0

#define I2C_MASTER 1
#if I2C_MASTER
    #define I2C_SLAVE 0
#else
    #define I2C_SLAVE 1
#endif
#define I2C_MASTER_ADDRESS 0x01
#define I2C_SLAVE_ADDRESS  0x02

#define LAYER_COUNT 6
#define ROW_PIN_COUNT 4
#define COL_PIN_COUNT 4
#define _ROW_PINS A3, A2, A1, A0
#define _COL_PINS 15, 14, 16, 10

const uint8_t ROW_PINS[ROW_PIN_COUNT] = { _ROW_PINS };
const uint8_t COL_PINS[COL_PIN_COUNT] = { _COL_PINS };

#endif
