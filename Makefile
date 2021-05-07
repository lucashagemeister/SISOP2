CC=g++ -std=c++11 -c
CFLAGS=-I./include/ -pthread

BIN_FOLDER=./bin/
SRC_FOLDER=./src/

DBFLAGS=-ggdb3 -O0
RELEASEFLAGS=-O2

SERVER_SRC=$(SRC_FOLDER)Client.cpp $(SRC_FOLDER)Packet.cpp $(SRC_FOLDER)Server.cpp $(SRC_FOLDER)Socket.cpp $(SRC_FOLDER)app_server.cpp
CLIENT_SRC=$(SRC_FOLDER)Client.cpp $(SRC_FOLDER)Packet.cpp $(SRC_FOLDER)Server.cpp $(SRC_FOLDER)Socket.cpp $(SRC_FOLDER)app_client.cpp

SERVER_OBJ=$(addprefix $(BIN_FOLDER),$(notdir $(SERVER_SRC:.cpp=.o)))
CLIENT_OBJ=$(addprefix $(BIN_FOLDER),$(notdir $(CLIENT_SRC:.cpp=.o)))

SERVER_EXE=./bin/app_server
CLIENT_EXE=./bin/app_client

server: $(SERVER_OBJ)
	g++ -pthread -o $(SERVER_EXE) $(SERVER_OBJ)  

client: $(CLIENT_OBJ)
	g++ -pthread -o $(CLIENT_EXE)  $(CLIENT_OBJ) 

./bin/%.o: ./src/%.cpp
	@mkdir -p $(BIN_FOLDER)
	$(CC) -o $@ $< $(CFLAGS) $(RELEASEFLAGS)

clean:
	rm -f $(BIN_FOLDER)*.o

all: server client