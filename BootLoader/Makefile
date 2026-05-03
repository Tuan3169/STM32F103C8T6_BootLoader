CC=arm-none-eabi-gcc
AS=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

CFLAGS=-mcpu=cortex-m3 -mthumb -O0 -g -Wall -ffreestanding $(INCLUDE_DIRS)
ASFLAGS=-mcpu=cortex-m3 -mthumb
LDFLAGS=-T Linker.ld -nostdlib

SRC_DIR=.
STARTUP_DIR=.

# 👇 nhận từ bên ngoài
BUILD_DIR ?= build

TARGET=bootloader

C_SOURCES=$(shell find $(SRC_DIR) -name "*.c")
ASM_SOURCES=$(shell find $(STARTUP_DIR) -name "*.s")

OBJECTS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES)) \
         $(patsubst $(STARTUP_DIR)/%.s,$(BUILD_DIR)/%.o,$(ASM_SOURCES))

all: $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(STARTUP_DIR)/%.s | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@
