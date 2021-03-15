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

	public:
		Socket();
		//Packet* readPacket(int client_socketfd, bool* connectedClient);
        //int sendPacket(int socket_fd, Packet mypacket);
};

#endif