#ifndef SOCKET_HEADER
#define SOCKET_HEADER

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "Packet.hpp"

class Socket
{
	private:
		int socketfd;

	public:
		int getSocketfd();
		
		Packet* readPacket();
        int sendPacket(Packet packet);
		
		Socket();
		Socket(int socketfd);
		~Socket();
};

class ClientSocket : public Socket {
	public:
		void connectToServer();
};


class ServerSocket : public Socket {
	
	public:
		struct sockaddr_in serv_addr;

		void bindAndListen();

		ServerSocket();

};

#endif