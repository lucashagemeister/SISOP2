#ifndef SOCKET_HEADER
#define SOCKET_HEADER

#include <stdint.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "Packet.hpp"

class Socket
{

	private:
		int sourceSocket;
		int destSocket;

	public:
		Packet* readPacket(bool* connectedClient);
        int sendPacket(Packet packet);

		Socket(int sourceSocket, int destSocket);
};

#endif