BOOT_ADDRESS = 0x8000000
BOOT_SIZE = 0x4000
APP_ADDRESS = 0x8004000
APP_SIZE = 0x6000
OTA_ADDRESS = 0x800A000
OTA_SIZE = 0x6000
APP_HASH_ADDRESS = 0x08009FD8
OTA_HASH_ADDRESS = 0x0800FFD8

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
ifeq ($(PLATFORM),Linux)
BOOT_BUILD = $(BUILD_ROOT)/boot
APP_BUILD  = $(BUILD_ROOT)/app
OTA_BUILD  = $(BUILD_ROOT)/OTA
LOG_FILE = $(ROOT_DIR)/BuildLog.log

BOOT_BIN = $(BOOT_BUILD)/Bootloader.bin
APP_BIN  = $(APP_BUILD)/Application.bin
OTA_BIN  = $(OTA_BUILD)/ApplicationOTA.bin
else
BOOT_BUILD = $(BUILD_ROOT)\boot
APP_BUILD  = $(BUILD_ROOT)\app
OTA_BUILD  = $(BUILD_ROOT)\OTA
LOG_FILE = $(ROOT_DIR)\BuildLog.log

BOOT_BIN = $(BOOT_BUILD)\Bootloader.bin
APP_BIN  = $(APP_BUILD)\Application.bin
OTA_BIN  = $(OTA_BUILD)\ApplicationOTA.bin
endif

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
# 	python3 $(ROOT_DIR)/BuildHashTool.py --bin $(APP_BIN) --total-size 0x6000 --crc-offset 0x5FFC --hash-offset 0x5FDC --hash-size 32
	./HashBinaryTool $(APP_BIN) $(BUILD_ROOT)/ApplicationHash.bin
	cp $(APP_BIN) $(BUILD_ROOT)/Application.bin

ota:
	$(MAKE) -C $(OTA_DIR) BUILD_DIR=../$(OTA_BUILD) INCLUDE_DIRS="$(INCLUDE_DIRS)" OTA_BIN=$(OTA_BIN)
	./HashBinaryTool $(OTA_BIN) $(BUILD_ROOT)/ApplicationOTAHash.bin
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

else

all: $(BUILD_ROOT) $(BOOT_BUILD) $(APP_BUILD) $(OTA_BUILD)
	@echo Building... > $(LOG_FILE)
	@echo Generating files from SDL... >> $(LOG_FILE)
# 	$(MAKE) gen >> $(LOG_FILE) 2>&1
	@echo Building Bootloader... >> $(LOG_FILE)
	$(MAKE) boot >> $(LOG_FILE) 2>&1
	@echo Building Application... >> $(LOG_FILE)
	$(MAKE) app >> $(LOG_FILE) 2>&1
	@echo Building OTA... >> $(LOG_FILE) 2>&1
	$(MAKE) ota >> $(LOG_FILE) 2>&1
	@echo Building complete. >> $(LOG_FILE)

# build bootloader
boot:
	$(MAKE) -C $(BOOT_DIR) BUILD_DIR=../$(BOOT_BUILD) INCLUDE_DIRS="$(INCLUDE_DIRS)" BOOT_BIN=$(BOOT_BIN)
	copy /Y $(BOOT_BIN) $(BUILD_ROOT)/Bootloader.bin

# build application
app:
	$(MAKE) -C $(APP_DIR) BUILD_DIR=../$(APP_BUILD) INCLUDE_DIRS="$(INCLUDE_DIRS)" APP_BIN=$(APP_BIN)
	python $(ROOT_DIR)/BuildHashTool.py --bin $(APP_BIN) --total-size 0x6000 --crc-offset 0x5FFC --hash-offset 0x5FDC --hash-size 32
	copy /Y $(APP_BIN) $(BUILD_ROOT)/Application.bin

# build OTA
ota:
	$(MAKE) -C $(OTA_DIR) BUILD_DIR=../$(OTA_BUILD) INCLUDE_DIRS="$(INCLUDE_DIRS)" OTA_BIN=$(OTA_BIN)
	copy /Y $(OTA_BIN) $(BUILD_ROOT)/ApplicationOTA.bin

# tạo thư mục
$(BUILD_ROOT):
	if not exist $(BUILD_ROOT) mkdir $(BUILD_ROOT)

$(BOOT_BUILD):
	if not exist $(BOOT_BUILD) mkdir $(BOOT_BUILD)

$(APP_BUILD):
	if not exist $(APP_BUILD) mkdir $(APP_BUILD)

$(OTA_BUILD):
	if not exist $(OTA_BUILD) mkdir $(OTA_BUILD)

# clean
clean:
	if exist $(BUILD_ROOT) rmdir /s /q $(BUILD_ROOT)

endif

# flash - only on Windows
ifeq ($(PLATFORM),Windows)
flash: flash-boot flash-app flash-ota flash-app-hash flash-ota-hash

flash-boot:
	@echo "Flashing bootloader via J-Link..."
	@echo erase > jlink_boot.cmd
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

flash-app-hash:
	@echo "Flashing app-hash via J-Link..."
	@echo loadfile $(BUILD_ROOT)/ApplicationHash.bin $(APP_HASH_ADDRESS) > jlink_app_hash.cmd
	@echo exit >> jlink_app_hash.cmd
	$(JLINK_EXE) -device $(JLINK_DEVICE) -if $(JLINK_IF) -speed $(JLINK_SPEED) -CommanderScript jlink_app_hash.cmd
	@del jlink_app_hash.cmd

flash-ota:
	@echo "Flashing OTA application via J-Link..."
	@echo loadfile $(OTA_BIN) 0x0800A000 > jlink_ota.cmd
	@echo exit >> jlink_ota.cmd
	$(JLINK_EXE) -device $(JLINK_DEVICE) -if $(JLINK_IF) -speed $(JLINK_SPEED) -CommanderScript jlink_ota.cmd
	@del jlink_ota.cmd

flash-ota-hash:
	@echo "Flashing ota-hash via J-Link..."
	@echo loadfile $(BUILD_ROOT)/ApplicationOTAHash.bin $(OTA_HASH_ADDRESS) > jlink_ota_hash.cmd
	@echo exit >> jlink_ota_hash.cmd
	$(JLINK_EXE) -device $(JLINK_DEVICE) -if $(JLINK_IF) -speed $(JLINK_SPEED) -CommanderScript jlink_ota_hash.cmd
	@del jlink_ota_hash.cmd

else
flash flash-boot flash-app flash-ota flash-app-hash:
	@echo "ERROR: Flash target is only available on Windows"
	@echo "You are running on $(PLATFORM). Flash commands require Windows with J-Link."
	@exit 1
endif

