#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "include/Packet.hpp"
#include "include/Socket.hpp"

#define PORT 20000

int main(int argc, char *argv[])
{
	int sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);     
    
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");
	
	listen(sockfd, 5);
	std::cout << "Listening...\n";
	
	clilen = sizeof(struct sockaddr_in);
	if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
		printf("ERROR on accept");
	
	

	Socket connectedSocket = Socket(newsockfd);
	//Packet* receivedPacket = connectedSocket.readPacket();
	Packet* rpkt = connectedSocket.readPacket();

	if (rpkt != NULL){
		std::cout << "Received message: " << rpkt->getPayload() << std::endl;
		std::cout << "Packet type: " << rpkt->getType()
		<< "\nPacket seqn: " << rpkt->getSeqn()
		<< "\nPacket timestamp: " << rpkt->getTimestamp()
		<< "\nPayload len: " << rpkt->getLength() << std::endl;
	}

	Packet newPacket = Packet(1, "I got your message ;)");
	connectedSocket.sendPacket(newPacket);

	close(sockfd);
	close(newsockfd);
	return 0; 
}