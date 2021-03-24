#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string> 
#include "include/Packet.hpp"
#include "include/Socket.hpp"
#include "include/defines.hpp"


using namespace std;


int main() {

    ClientSocket socket = ClientSocket();
    socket.connectToServer();
	Packet pkt(1, "Hi from client, this is a payload!");
	

	while (1)
	{
		socket.sendPacket(pkt);

		/*
		Packet* rpkt = socket.readPacket();
		std::cout << "Got server message: " << rpkt->getPayload() << endl;
		std::cout << "Packet type: " << rpkt->getType()
		<< "\nPacket seqn: " << rpkt->getSeqn()
		<< "\nPacket timestamp: " << rpkt->getTimestamp()
		<< "\nPayload len: " << rpkt->getLength() << endl;

		std::cout << "\n\n";*/
		sleep(5);
	}

	return 0;
}