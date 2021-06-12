# Usage:
# make										# compile the program
# make clean								# remove previous build
# make run FILE1=file1Path FILE2=file2Path	# clean, compile and run the program

CXX	:= gcc
CXX_FLAGS := -std=c11 -ggdb -w

BIN	:= bin
SRC	:= src
INCLUDE	:= include

LIBRARIES   := -lm -pthread
EXECUTABLE	:= main

FILES := files

# default arguments
FILE1 := $(FILES)/file1
FILE2 := $(FILES)/file2

all: $(BIN)/$(EXECUTABLE)

run: clean all
		clear -x
		@echo "Executing files..."
		./$(BIN)/$(EXECUTABLE) $(FILE1) $(FILE2)

$(BIN)/$(EXECUTABLE): $(SRC)/*.c
		@echo "Compiling the program..."
		$(CXX) $(CXX_FLAGS) -I$(INCLUDE) $^ -o $@ $(LIBRARIES)
		@echo "Program compiled!"

clean:
		@echo "Cleaning up..."
		-rm $(BIN)/* -f
