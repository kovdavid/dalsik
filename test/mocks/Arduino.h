#pragma once

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PROGMEM
#define HEX 16
#define BIN 2

#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(string_literal))
#define UNUSED __attribute__((unused))

#define USBCON (0x65)
#define OTGPADE (0x01)

class __FlashStringHelper;

class Serial_CLS {
   public:
      void print( int value );
      void print( uint8_t value );
      void print( uint8_t value, int );
      void print( unsigned char*, int );
      void print( unsigned long );
      void print( const char* );
      void print( const __FlashStringHelper* );
      void println( unsigned char* );
};

extern Serial_CLS Serial;
