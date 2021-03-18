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
#define PORT 20000


using namespace std;


int main() {

    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	
	server = gethostbyname("127.0.0.1");
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("ERROR opening socket\n");
    
	serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     
	

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        printf("ERROR connecting\n");

	
	Packet pkt(1, "Hi from client, this is a payload!");
	Socket socket = Socket(sockfd);
	socket.sendPacket(pkt);

	Packet* rpkt = socket.readPacket();
	std::cout << "Got server message: " << rpkt->getPayload() << endl;
	std::cout << "Packet type: " << rpkt->getType()
	<< "\nPacket seqn: " << rpkt->getSeqn()
	<< "\nPacket timestamp: " << rpkt->getTimestamp()
	<< "\nPayload len: " << rpkt->getLength() << endl;

	close(sockfd);
	return 0;
}