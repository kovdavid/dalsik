ARDUINO_DIR    = /usr/share/arduino

BOARD_TAG         = promicro
BOARD_SUB         = 16MHzatmega32U4
ALTERNATE_CORE    = sparkfun
ARDMK_VENDOR      = sparkfun
BOARDS_TXT        = $(ARDUINO_DIR)/hardware/sparkfun/avr/boards.txt
BOOTLOADER_PARENT = $(ARDUINO_DIR)/hardware/sparkfun/avr/bootloaders
ARDUINO_CORE_PATH = $(ARDUINO_DIR)/hardware/archlinux-arduino/avr/cores/arduino
BOOTLOADER_PATH   = caterina
BOOTLOADER_FILE   = Caterina-promicro16.hex
ISP_PROG          = usbasp
AVRDUDE_OPTS      = -v
USER_LIB_PATH     = /opt/avr/packages/arduino/hardware/avr/1.6.20/libraries
ARDUINO_LIBS      = EEPROM HID

include /home/davs/workspace/avr/Arduino-Makefile/Arduino.mk
