
BUILD_ROOT = Build
ROOT_DIR := $(CURDIR)

# Detect OS
ifeq ($(OS),Windows_NT)
  PLATFORM = Windows
else
  PLATFORM = Linux
endif

JLINK_EXE = "F:\SEGGER\JLink_V898\JLink.exe"
GEN_SCRIPT = BuildSdlTool.py

BOOT_DIR = bootloader
APP_DIR  = application
OTA_DIR  = OTA

BOOT_BUILD = $(BUILD_ROOT)/boot
APP_BUILD  = $(BUILD_ROOT)/app
OTA_BUILD  = $(BUILD_ROOT)/OTA
LOG_FILE = $(BUILD_ROOT)/BuildLog.log

BOOT_BIN = $(BOOT_BUILD)/Bootloader.bin
APP_BIN  = $(APP_BUILD)/Application.bin
OTA_BIN  = $(OTA_BUILD)/ApplicationOTA.bin

INCLUDE_DIRS=-I$(ROOT_DIR)	\
				-I$(ROOT_DIR)/Stm32Pkg/stm32f103/Inc

MERGED = $(BUILD_ROOT)/firmware.bin
# J-Link configuration
JLINK_DEVICE = STM32F103C8
JLINK_IF = SWD
JLINK_SPEED = 4000
JLINK_SCRIPT = flash.jlink

.PHONY: all boot app ota clean flash flash-boot flash-app flash-ota

ifeq ($(PLATFORM),Linux)
	
all: $(BUILD_ROOT) $(BOOT_BUILD) $(APP_BUILD) $(OTA_BUILD)
	@echo "Building..." 2>&1 | tee -a $(LOG_FILE)
	@echo "Generating files from SDL..." 2>&1 | tee -a $(LOG_FILE)
	$(MAKE) gen 2>&1 | tee -a $(LOG_FILE)
	@echo "Building Bootloader..." 2>&1 | tee -a $(LOG_FILE)
	$(MAKE) boot 2>&1 | tee -a $(LOG_FILE)
	@echo "Building Application..."
	$(MAKE) app 2>&1 | tee -a $(LOG_FILE)
	@echo "Building OTA..." 2>&1 | tee -a $(LOG_FILE)
	$(MAKE) ota 2>&1 | tee -a $(LOG_FILE)
	@echo "Building complete." 2>&1 | tee -a $(LOG_FILE)


# build bootloader
boot:
	$(MAKE) -C $(BOOT_DIR) BUILD_DIR=../$(BOOT_BUILD) INCLUDE_DIRS="$(INCLUDE_DIRS)" BOOT_BIN=$(BOOT_BIN)
	cp $(BOOT_BIN) $(BUILD_ROOT)/Bootloader.bin

# build app
app:
	$(MAKE) -C $(APP_DIR) BUILD_DIR=../$(APP_BUILD) INCLUDE_DIRS="$(INCLUDE_DIRS)" APP_BIN=$(APP_BIN)
	python3 $(ROOT_DIR)/BuildHashTool.py --bin $(APP_BIN) --total-size 0x6000 --crc-offset 0x5FF8 --hash-offset 0x5FFC --hash-size 4
	cp $(APP_BIN) $(BUILD_ROOT)/Application.bin

ota:
	$(MAKE) -C $(OTA_DIR) BUILD_DIR=../$(OTA_BUILD) INCLUDE_DIRS="$(INCLUDE_DIRS)" OTA_BIN=$(OTA_BIN)
	cp $(OTA_BIN) $(BUILD_ROOT)/ApplicationOTA.bin

# tạo thư mục
$(BUILD_ROOT):
	mkdir -p $(BUILD_ROOT)

$(BOOT_BUILD):
	mkdir -p $(BOOT_BUILD)

$(APP_BUILD):
	mkdir -p $(APP_BUILD)

$(OTA_BUILD):
	mkdir -p $(OTA_BUILD)

# clean
clean:
	rm -rf $(BUILD_ROOT)

endif

# flash - only on Windows
ifeq ($(PLATFORM),Windows)
flash: flash-boot flash-app

flash-boot:
	@echo "Flashing bootloader via J-Link..."
	@echo loadfile $(BUILD_ROOT)/Bootloader.bin 0x08000000 > jlink_boot.cmd
	@echo exit >> jlink_boot.cmd
	$(JLINK_EXE) -device $(JLINK_DEVICE) -if $(JLINK_IF) -speed $(JLINK_SPEED) -autoconnect 1 -CommanderScript jlink_boot.cmd
	@del jlink_boot.cmd

flash-app:
	@echo "Flashing application via J-Link..."
	@echo loadfile $(APP_BIN) 0x08004000 > jlink_app.cmd
	@echo exit >> jlink_app.cmd
	$(JLINK_EXE) -device $(JLINK_DEVICE) -if $(JLINK_IF) -speed $(JLINK_SPEED) -CommanderScript jlink_app.cmd
	@del jlink_app.cmd

flash-ota:
	@echo "Flashing OTA application via J-Link..."
	@echo loadfile $(OTA_BIN) 0x0800A000 > jlink_ota.cmd
	@echo exit >> jlink_ota.cmd
	$(JLINK_EXE) -device $(JLINK_DEVICE) -if $(JLINK_IF) -speed $(JLINK_SPEED) -CommanderScript jlink_ota.cmd
	@del jlink_ota.cmd

else
flash flash-boot flash-app flash-ota:
	@echo "ERROR: Flash target is only available on Windows"
	@echo "You are running on $(PLATFORM). Flash commands require Windows with J-Link."
	@exit 1
endif

