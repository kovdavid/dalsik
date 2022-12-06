### DISCLAIMER
### This is an example Makefile and it MUST be configured to suit your needs.
### For detailed explanations about all the available options,
### please refer to https://github.com/sudar/Arduino-Makefile/blob/master/arduino-mk-vars.md

### PROJECT_DIR
### This is the path to where you have created/cloned your project
PROJECT_DIR       = $(shell git root)

### ARDMK_DIR
### Path to the Arduino-Makefile directory.
ARDMK_DIR         = $(PROJECT_DIR)/Arduino-Makefile

### ARDUINO_DIR

### Path to the Arduino application and resources directory.
# ARDUINO_DIR       = // set by calling make

### USER_LIB_PATH
### Path to where the your project's libraries are stored.
USER_LIB_PATH    :=  $(realpath $(PROJECT_DIR)/lib)

### BOARD_TAG
### It must be set to the board you are currently using. (i.e uno, mega2560, etc.)
BOARD_TAG         = leonardo
BOARD_SUB         = atmega32u4

### MONITOR_PORT
### The port your board is connected to. Using an '*' tries all the ports and finds the right one.
MONITOR_PORT      = /dev/ttyACM*

### MONITOR_BAUDRATE
### It must be set to Serial baudrate value you are using.
MONITOR_BAUDRATE  = 115200

### AVR_TOOLS_DIR
### Path to the AVR tools directory such as avr-gcc, avr-g++, etc.
# AVR_TOOLS_DIR     = // set by calling make

### AVRDUDE
### Path to avrdude directory.
AVRDUDE          = /usr/bin/avrdude
AVRDUDE_CONF     = /etc/avrdude.conf

### CFLAGS_STD
CFLAGS_STD        = -std=gnu11

### CXXFLAGS_STD
CXXFLAGS_STD      = -std=gnu++14

### CPPFLAGS
### Flags you might want to set for debugging purpose. Comment to stop.
CXXFLAGS         += -pedantic -Wall -Wextra -Wfatal-errors  ## -Werror

ifdef DALSIK_DEBUG
CXXFLAGS         += -D DEBUG -D DEBUG_KEYREPORT_BASE -D DEBUG_KEYBOARD_STATE
endif

ifdef DALSIK_REPORT_LAYER_CHANGE
CXXFLAGS         += -D REPORT_LAYER_CHANGE
endif

ifdef DALSIK_REPORT_MODIFIER_CHANGE
CXXFLAGS         += -D REPORT_MODIFIER_CHANGE
endif

LDFLAGS          += -fdiagnostics-color

### OBJDIR
### Don't touch this!
### This is were you put the binaries you just compile using 'make'
CURRENT_DIR       = $(shell basename $(CURDIR))
OBJDIR            = $(PROJECT_DIR)/build/$(CURRENT_DIR)/$(BOARD_TAG)

### Keymap setting
ifndef DALSIK_KEYMAP
$(error DALSIK_KEYMAP environment variable is not set)
endif

KEYMAP_TARGET = $(OBJDIR)/userlibs/keymap/keymap.cpp.o

COMMON_DEPS = $(KEYMAP_TARGET)

### path to Arduino.mk, inside the ARDMK_DIR, don't touch.
include $(ARDMK_DIR)/Arduino.mk

$(KEYMAP_TARGET): $(DALSIK_KEYMAP)
	@$(MKDIR) $(dir $@)
	$(CXX) -MMD -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

USER_LIB_OBJS += $(KEYMAP_TARGET)
