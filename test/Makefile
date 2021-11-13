PROJECT_ROOT := $(PWD)
BUILD_ROOT := $(PROJECT_ROOT)/build/test

CC = g++

CFLAGS =
CFLAGS += -g
CFLAGS += -std=gnu++14
CFLAGS += -w
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wfatal-errors
CFLAGS += -O0
CFLAGS += -fdata-sections
CFLAGS += -fdiagnostics-color=always
CFLAGS += -ffunction-sections
CFLAGS += -flto
CFLAGS += -fno-devirtualize
CFLAGS += -fno-threadsafe-statics
CFLAGS += -fpermissive
CFLAGS += -pedantic
CFLAGS += -include $(PROJECT_ROOT)/test/mocks/misc.h

INCLUDES =
INCLUDES += -I$(PROJECT_ROOT)/test/mocks
INCLUDES += -I$(PROJECT_ROOT)/test/acutest/include
INCLUDES += -I$(PROJECT_ROOT)/lib/dalsik/
INCLUDES += -I$(PROJECT_ROOT)/lib/master_report/
INCLUDES += -I$(PROJECT_ROOT)/lib/keymap/
INCLUDES += -I$(PROJECT_ROOT)/lib/matrix/
INCLUDES += -I$(PROJECT_ROOT)/lib/key_definitions/
INCLUDES += -I$(PROJECT_ROOT)/lib/dalsik_hid/
INCLUDES += -I$(PROJECT_ROOT)/lib/dalsik_hid_desc/
INCLUDES += -I$(PROJECT_ROOT)/lib/dalsik_eeprom/
INCLUDES += -I$(PROJECT_ROOT)/lib/array_utils/

COMPILER := $(CC) $(CFLAGS) $(INCLUDES)

$(BUILD_ROOT)/%.o: $(PROJECT_ROOT)/test/%
	$(COMPILER) -c $< -o $@

$(BUILD_ROOT)/%.o: $(PROJECT_ROOT)/lib/*/%
	$(COMPILER) -c $< -o $@

$(BUILD_ROOT)/%.o: $(PROJECT_ROOT)/test/*/%
	$(COMPILER) -c $< -o $@

OBJ = test_master_report.cpp.o mock_eeprom.cpp.o dalsik_eeprom.cpp.o master_report.cpp.o keymap.cpp.o array_utils.cpp.o HID.cpp.o Serial.cpp.o

test_master_report: $(addprefix $(BUILD_ROOT)/, $(OBJ))
	echo ">> Linking $(BUILD_ROOT)/test_master_report"
	$(COMPILER) $^ -o $(BUILD_ROOT)/test_master_report

create_build_directory:
	rm -rf $(PROJECT_ROOT)/build
	mkdir -p $(BUILD_ROOT)

test: create_build_directory test_master_report
	$(BUILD_ROOT)/test_master_report

# https://www.youtube.com/watch?v=NPdagdEOBnI
# https://github.com/JeremiahK96/auto-makefile/blob/main/Makefile
