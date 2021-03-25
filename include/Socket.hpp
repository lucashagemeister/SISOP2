#ifndef SOCKET_HEADER
#define SOCKET_HEADER

#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include "Packet.hpp"
#include "./Server.hpp"
using namespace std;

class Socket
{
	private:
		int socketfd;

	public:
		int getSocketfd();
		
		Packet* readPacket();
        int sendPacket(Packet packet);
		int sendPacket(Packet pkt, int socketfd);
		
		Socket();
		Socket(int socketfd);
		~Socket();
};

class ClientSocket : public Socket {
	public:
		void connectToServer();
		void connectToServer(string serverAddress, int serverPort);
};


class ServerSocket : public Socket {
	
	public:
		struct sockaddr_in serv_addr;

		void bindAndListen();
		void connectNewClient(pthread_t *threadID,  void *(*communicationHandler)(void*), Server server);

		ServerSocket();

};


struct communiction_handler_args {
	int connectedSocket;
	host_address client_address; 
	string user;
};


#endif