PROJECT := appweeb
MODE ?= debug
PLATFORM ?= windows
TARGET_NAME = $(PROJECT)

ifeq ($(PLATFORM), linux)
	DEFINES = 
	DEPENDENCIES = 
	LIBRARIES = 
else ifeq ($(PLATFORM), windows)
	DEFINES = _WIN32
	DEPENDENCIES =  
	LIBRARIES = ws2_32
endif

# Project directories
PRJ_DIR = ./
SRC_DIR = $(PRJ_DIR)src
DEP_DIR = $(PRJ_DIR)lib
BIN_DIR = $(PRJ_DIR)bin/$(PLATFORM)/$(MODE)
OBJ_DIR = $(PRJ_DIR)bin/$(PLATFORM)/obj/$(MODE)/$(TARGET_NAME)
DLL_DIR = $(PRJ_DIR)lib/bin/$(PLATFORM)/$(MODE)

# Include directories
INC_DIR := $(PRJ_DIR)/src 
INC_DIR += $(PRJ_DIR)lib

# Format libraries and includes as parameters for the compiler
CLIBS := $(foreach LIB,$(LIBRARIES),-l$(LIB))
CINCS := $(foreach INC,$(INC_DIR),-I$(INC))
CC = g++

# Compiler flags
CFLAGS = -Wall $(CINCS) -Wno-deprecated-declarations -std=c++23 -Werror -MMD -MP -Wfatal-errors -Wextra
ifeq ($(MODE), release)
	CFLAGS += --static -static-libgcc -static-libstdc++ -Wno-error=free-nonheap-object -O3 -Wl,--as-needed
else
	CFLAGS += -g -O0 -Wno-error=unused-but-set-variable -Wno-error=unused-variable 
	DEFINES += DEBUG
endif

# Source files and corresponding object files
SRC := $(wildcard $(SRC_DIR)/*.cpp) 
SRC += $(wildcard $(SRC_DIR)/platform/$(PLATFORM)/*.cpp)	

OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

.PHONY: all clean run rebuild check

all: $(BIN_DIR)/$(TARGET_NAME) check

$(BIN_DIR)/$(TARGET_NAME): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(foreach LIB,$(DEPENDENCIES),make -C $(DEP_DIR)/$(LIB) MODE=$(MODE) PLATFORM=$(PLATFORM);)
ifeq ($(PLATFORM), windows)
	# Copy required DLLs for Windows platform
endif

	# Link the object files into the final executable
	$(CC) $(CFLAGS) $(foreach DEF,$(DEFINES),-D$(DEF)) -o $@ $^ $(foreach DIR,$(DLL_DIR),-L$(DIR)) $(CLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(@D)
	# Compile source file into object file and generate dependency file
	$(CC) $(CFLAGS) $(foreach DEF,$(DEFINES),-D$(DEF)) -c -o $@ -MMD -MP $<

clean:
	$(foreach LIB,$(DEPENDENCIES),make -C $(DEP_DIR)/$(LIB) clean MODE=$(MODE);)
	# Remove object files and binaries
	@rm -rf $(OBJ_DIR)
	@rm -rf $(BIN_DIR)

run: $(BIN_DIR)/$(TARGET_NAME)
	$(BIN_DIR)/$(TARGET_NAME)

rebuild: clean all

$(OBJ_DIR):
	@mkdir -p $@

-include $(DEP)

