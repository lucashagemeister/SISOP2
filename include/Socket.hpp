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
#include <string>
#include "Packet.hpp"
using namespace std;

typedef struct address {
	string ipv4;
	int port;

    bool operator ==(address other) const {
		return ipv4 == other.ipv4 && port == other.port;
	}

    bool operator <(const address& other) const {
		return port < other.port;
	}
    
} host_address;



class Socket
{
	private:
		int socketfd;

	public:
		int getSocketfd();
		
		Packet* readPacket();
		static Packet* readPacket(int socketfd);
        int sendPacket(Packet packet);
		int sendPacket(Packet pkt, int socketfd);
		
		Socket();
		Socket(int socketfd);
		~Socket();
};

class ClientSocket : public Socket {
	public:
		void connectToServer();
		void connectToServer(const char* serverAddress, int serverPort);
};


#endif