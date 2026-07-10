PROJECT := upload
MODE ?= debug
PLATFORM ?= windows

SRC_DIR = .
BIN_DIR = ../../bin/$(PLATFORM)/$(MODE)/plugins

OBJ_DIR = ../../bin/$(PLATFORM)/obj/$(MODE)/$(PROJECT)

INC_DIR := ../../include
INC_DIR += ../../src
INC_DIR += ../../lib

CC = g++

CINCS := $(foreach INC,$(INC_DIR),-I$(INC))

CFLAGS = -Wall $(CINCS) -std=c++26 -Werror -MMD -MP -Wextra

ifeq ($(PLATFORM), windows)
    SHARED_EXT = dll
else ifeq ($(PLATFORM), linux)
    SHARED_EXT = so
	CC := g++-16
endif

ifeq ($(MODE), release)
    CFLAGS += -O3
else
    CFLAGS += -g -O0
endif

SRC := $(wildcard $(SRC_DIR)/*.cpp)

OBJ := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

TARGET = $(BIN_DIR)/$(PROJECT).$(SHARED_EXT)

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) -shared -o $@ $^

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)

-include $(DEP)