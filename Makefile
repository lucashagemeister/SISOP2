CC := g++

LIB_DIR := lib
INC_DIR := include
OBJ_DIR := bin
SRC_DIR := src

INC := -I$(INC_DIR)
LIB := -L$(LIB_DIR)

BUILD_DIR := build

CFLAGS += -pthread

MAIN := $(app_client.cpp)

TARGET_EXE := $(BUILD_DIR)/%

SRC := $(shell find $(SRC_DIR) -name '*.cpp')

OBJ := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SRC)))

F ?= app_client

$(TARGET_EXE): $(SRC_DIR)/*.cpp $(OBJ)
		$(CC) -o $@ $^ $(INC) $(CFLAGS) $(LIB)

$(OBJ_DIR)/%.o: ./*cpp
	$(CC) -o $@ $^ $(SRC) $(INC) $(CFLAGS) $(LIB)

.PHONY: all clean

.DEFAULT_GOAL: all

all: $(TARGET_EXE)

run: $(BUILD_DIR)/$(F)

clean: rm -f $(OBJ_DIR)/*.o $(TARGET_EXE)