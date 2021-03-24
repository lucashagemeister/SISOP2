#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "include/Packet.hpp"
#include "include/Socket.hpp"
#include "include/defines.hpp"
    


void *communicationHandlerExample(void *handlerArgs){

	struct communiction_handler_args *args = (struct communiction_handler_args *)handlerArgs;
	Socket connectedSocket = Socket(args->connectedSocket);

	int i=100;
	while (1){
		
		Packet* rpkt = connectedSocket.readPacket();

		if (rpkt != NULL){
			std::cout << "Received message: " << rpkt->getPayload() << std::endl;
			std::cout << "Packet type: " << rpkt->getType()
			<< "\nPacket seqn: " << rpkt->getSeqn()
			<< "\nPacket timestamp: " << rpkt->getTimestamp()
			<< "\nPayload len: " << rpkt->getLength() << "\n\n";
		} else {
			return 0;
		}

		Packet newPacket = Packet(i, "I got your message ;)");
		int n = connectedSocket.sendPacket(newPacket);
		if (n < 0) {
			return 0;
		}

		i++;
	}
	return 0;
}


int main(int argc, char *argv[])
{
	pthread_t threadConnections[MAX_TCP_CONNECTIONS];
	int i = 0;

	ServerSocket socket = ServerSocket();
	socket.bindAndListen();

	while (1){
		socket.connectNewClient(&threadConnections[i], communicationHandlerExample);
		i++;
	}

	return 0; 
}