# compiler
CC = gcc

# target
TARGET = HashTool.exe

# directories
ROOT_DIR = ../..
LIB_DIR = $(ROOT_DIR)/Library
BUILD_DIR = build
DIR = .

SOURCES_LIBRARY = $(wildcard $(LIB_DIR)/Crc32/*.*) \
				  $(wildcard $(LIB_DIR)/Hash256/*.*)

# source files
# # SRCS = $(wildcard $(LIB_DIR)/Crc32/*.c) \
# # 	   $(wildcard $(LIB_DIR)/Hash256/*.c) \
# 	   $(wildcard *.c)

SRCS = $(wildcard *.c)

# object files
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

# include paths
INCLUDES = -I$(LIB_DIR)

# flags
CFLAGS = -Wall -O2 $(INCLUDES)

# default target
all: $(BUILD_DIR) $(TARGET)

# link
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# compile
$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# create build dir
$(BUILD_DIR):
	if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	copy 

# clean
clean:
	if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	if exist "$(TARGET)" del "$(TARGET)"

.PHONY: all clean