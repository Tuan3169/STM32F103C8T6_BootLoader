# Detect OS
ifeq ($(OS),Windows_NT)
  PLATFORM = Windows
else
  PLATFORM = Linux
endif

ifeq ($(PLATFORM),Linux)
CC=arm-none-eabi-gcc
AS=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
else
ARM_GNU_TOOLCHAIN_PATH = "F:\arm-none-eabi\bin"
CC=$(ARM_GNU_TOOLCHAIN_PATH)/arm-none-eabi-gcc.exe
AS=$(ARM_GNU_TOOLCHAIN_PATH)/arm-none-eabi-gcc.exe
OBJCOPY=$(ARM_GNU_TOOLCHAIN_PATH)/arm-none-eabi-objcopy.exe
endif

STM32PKG_DIR ?= ../Stm32Pkg/stm32f103
ROOT_DIR ?= ..

CFLAGS=-mcpu=cortex-m3 -mthumb -O0 -g -Wall -ffreestanding $(INCLUDE_DIRS)
ASFLAGS=-mcpu=cortex-m3 -mthumb
LDFLAGS=-T Linker.ld -nostdlib

SRC_DIR=.
STARTUP_DIR=.

# 👇 nhận từ bên ngoài
BUILD_DIR ?= build

TARGET=bootloader

LOCAL_C_SOURCES=$(shell find $(SRC_DIR) -name "*.c")
ROOT_C_SOURCES=$(ROOT_DIR)/boot_flag.c
PKG_C_SOURCES=$(shell find $(STM32PKG_DIR)/Src -name "*.c")
ASM_SOURCES=$(shell find $(STARTUP_DIR) -name "*.s")

LOCAL_OBJECTS=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LOCAL_C_SOURCES))
ROOT_OBJECTS=$(patsubst $(ROOT_DIR)/%.c,$(BUILD_DIR)/root/%.o,$(ROOT_C_SOURCES))
PKG_OBJECTS=$(patsubst $(STM32PKG_DIR)/Src/%.c,$(BUILD_DIR)/stm32pkg/%.o,$(PKG_C_SOURCES))
ASM_OBJECTS=$(patsubst $(STARTUP_DIR)/%.s,$(BUILD_DIR)/%.o,$(ASM_SOURCES))

OBJECTS=$(LOCAL_OBJECTS) $(ROOT_OBJECTS) $(PKG_OBJECTS) $(ASM_OBJECTS)

all: $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(BUILD_DIR)/stm32pkg/%.o: $(STM32PKG_DIR)/Src/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(BUILD_DIR)/root/%.o: $(ROOT_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(BUILD_DIR)/%.o: $(STARTUP_DIR)/%.s | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@
