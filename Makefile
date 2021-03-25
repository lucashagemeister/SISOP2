CC=g++ -std=c++11 -c
CFLAGS=-I./include/

BIN_FOLDER=./bin/
SRC_FOLDER=./src/

DBFLAGS=-ggdb3 -O0
RELEASEFLAGS=-O2

SRC=$(wildcard $(SRC_FOLDER)*.cpp) $(wildcard ./*.cpp)
OBJ=$(addprefix $(BIN_FOLDER),$(notdir $(SRC:.cpp=.o)))
EXE=./app_client.cpp

all: $(OBJ)
	g++ -o $(EXE) $(OBJ)

./bin/%.o: ./src/%.cpp
	@mkdir -p $(BIN_FOLDER)
	$(CC) -o $@ $< $(CFLAGS) $(RELEASEFLAGS)

clean:
	rm -f $(BIN_FOLDER)*.o

