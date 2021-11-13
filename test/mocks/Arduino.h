#pragma once

#include "misc.h"

#define USBCON (0x65)
#define OTGPADE (0x01)

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
