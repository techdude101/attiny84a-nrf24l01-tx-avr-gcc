# -------------------------------------------------------
# MCU / Project Configuration (inserted by generator)
# -------------------------------------------------------

MCU       = attiny84a
F_CPU     = 1000000
PARTNO    = t84a
PROG      = avrispmkII
PORT      = usb
BITCLOCK  = 5
IO_DEF    = __AVR_ATtiny84A__

# Firmware name (fixed as specified)
TARGET    = firmware

# -------------------------------------------------------
# Directory Structure
# -------------------------------------------------------

BUILD_DIR = .build
OBJ_DIR   = $(BUILD_DIR)/src

SRC_DIRS  = src lib
INC_DIRS  = include \
            $(shell find lib -type d)

INC_FLAGS = $(foreach d, $(INC_DIRS), -I$(d))

# -------------------------------------------------------
# Source Discovery (recursive)
# -------------------------------------------------------

SRC := $(wildcard src/*.c) \
       $(shell find lib -name '*.c')

OBJ_DIR := .build
OBJ := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))

# -------------------------------------------------------
# Tools
# -------------------------------------------------------

CC      = avr-gcc
OBJCOPY = avr-objcopy

# -------------------------------------------------------
# Compiler / Linker Flags
# -------------------------------------------------------

#CFLAGS  = -mmcu=$(MCU) \
#          -DF_CPU=$(F_CPU) \
#          -D$(IO_DEF) \
#          -Os $(INC_FLAGS) \
#          -std=gnu11 \
#          -fno-fat-lto-objects \
#          -Os \
#          -Wall \
#          -ffunction-sections \
#          -fdata-sections \
#          -flto

CFLAGS  = -mmcu=$(MCU) \
          -D$(IO_DEF) \
          -Os $(INC_FLAGS) \
          -std=gnu11 \
          -fno-fat-lto-objects \
          -Os \
          -Wall \
          -ffunction-sections \
          -fdata-sections \
          -flto

LDFLAGS = -mmcu=$(MCU)

# -------------------------------------------------------
# Pretty Printing
# -------------------------------------------------------

BLUE  := \033[38;5;14m
GREEN := \033[38;5;34m
YELLOW := \033[1;33m
CYAN := \033[1;36m
MAGENTA := \033[38;5;213m
ORANGE := \033[38;5;208m
RED := \033[38;5;196m

C_COMPILE := \033[38;5;39m
C_LINK    := \033[38;5;82m
C_BUILD   := \033[38;5;141m
C_FLASH   := \033[38;5;208m

RESET := \033[0m

define PRINT_CC
	@echo "$(C_COMPILE)[Compile]$(RESET) $<"
endef

define PRINT_LD
	@echo "$(C_LINK)[Link]$(RESET) $@"
endef

define PRINT_HEX
	@echo "$(C_BUILD)[Build]$(RESET) $@"
endef

define PRINT_FLASH
	@echo "$(C_FLASH)[Flash]$(RESET) $(BUILD_DIR)/$(TARGET).hex"
endef


# -------------------------------------------------------
# Default target
# -------------------------------------------------------

all: $(BUILD_DIR)/$(TARGET).hex

# -------------------------------------------------------
# Build Pattern Rules
# -------------------------------------------------------

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(PRINT_CC)
	@$(CC) $(CFLAGS) -c $< -o $@

# -------------------------------------------------------
# Linking
# -------------------------------------------------------

$(BUILD_DIR)/$(TARGET).elf: $(OBJ)
	$(PRINT_LD)
	@$(CC) $(LDFLAGS) $^ -o $@

# -------------------------------------------------------
# HEX output
# -------------------------------------------------------

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(PRINT_HEX)
	@$(OBJCOPY) -O ihex -R .eeprom $< $@

# -------------------------------------------------------
# Flashing
# -------------------------------------------------------

flash: $(BUILD_DIR)/$(TARGET).hex
	$(PRINT_FLASH)
	@echo ; \
	avrdude -p $(PARTNO) -c $(PROG) -B $(BITCLOCK) -P ${PORT} \
		-U flash:w:$(BUILD_DIR)/$(TARGET).hex:i 2>&1 \
       | grep -v '^Set SCK'; echo "$(RESET)" 

# -------------------------------------------------------
# Cleanup
# -------------------------------------------------------

clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: all flash clean
