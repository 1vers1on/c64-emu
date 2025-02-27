CC := gcc
CXX := g++
AS := nasm
TARGET ?= main
BUILD_TYPE ?= debug
INCLUDE_DIR := include
BIN_DIR := bin
SRC_DIR := src
SRC_DIRS := $(shell find $(SRC_DIR) -type d)
CFLAGS := -std=c11 -Werror -g -I$(INCLUDE_DIR) -MMD -MP
CXXFLAGS := -std=c++17 -Werror -g -I$(INCLUDE_DIR) -MMD -MP
LDFLAGS := -lm -lSDL2
ifeq ($(BUILD_TYPE), debug)
CFLAGS += -O0 -DDEBUG
CXXFLAGS += -O0 -DDEBUG
else
CFLAGS += -O2
CXXFLAGS += -O2
endif
CPP_SRC := $(shell find $(SRC_DIR) -name '*.cpp')
C_SRC := $(shell find $(SRC_DIR) -name '*.c')
ASM_SRC := $(shell find $(SRC_DIR) -name '*.asm')
CPP_OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%.o,$(CPP_SRC))
C_OBJ := $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(C_SRC))
ASM_OBJ := $(patsubst $(SRC_DIR)/%.asm,$(BIN_DIR)/%.o,$(ASM_SRC))
OBJ := $(CPP_OBJ) $(C_OBJ) $(ASM_OBJ)
DEP := $(OBJ:.o=.d)
all: $(TARGET)
$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
lib: $(OBJ)
	ar rcs $(BIN_DIR)/lib$(TARGET).a $(OBJ)
shared: $(OBJ)
	$(CXX) -shared -o $(BIN_DIR)/lib$(TARGET).so $(OBJ)
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
$(BIN_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AS) -f elf64 -o $@ $<
clean:
	rm -rf $(BIN_DIR) $(TARGET)
help:
	@echo "Available targets:"
	@echo "  all       - Build the project (default)"
	@echo "  clean     - Remove build artifacts"
	@echo "  lib       - Build a static library"
	@echo "  shared    - Build a shared library"
	@echo "  help      - Show this help message"
-include $(DEP)
.PHONY: all clean lib shared help