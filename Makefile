CC = g++
CC_FLAGS = -Wall -Wextra -ggdb -Wno-unused-parameter

BUILD_DIR = build
SOURCE_DIR = src

_OBJFILES = main.o cpu.o ram.o rom.o
OBJFILES = $(patsubst %,$(BUILD_DIR)/%,$(_OBJFILES))

all: nesemu 

nesemu: $(OBJFILES)
	$(CC) $(CC_FLAGS) -o $@ $^

$(BUILD_DIR)/main.o: $(SOURCE_DIR)/main.cpp
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CC_FLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(SOURCE_DIR)/%.h
	$(CC) $(CC_FLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
