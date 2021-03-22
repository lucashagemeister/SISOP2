#include "../include/Socket.hpp"


Socket::Socket(){
    if ((this->socketfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        std::cout << "ERROR opening socket\n" << std::endl;
        exit(1);
    }
}

Socket::Socket(int socketfd){
    this->socketfd = socketfd;
}


Socket::~Socket(){
    std::cout << "Closing socketfd...\n";
    close(this->socketfd);
}


int Socket::getSocketfd(){
    return this->socketfd;
}


Packet* Socket::readPacket(){

    Packet* pkt = new Packet();
    memset(pkt, 0, sizeof (Packet));
    int n = read(this->socketfd, pkt, sizeof(Packet));

    if (n<0){
        std::cout << "ERROR reading from socket" << std::endl;
        return NULL;
    }
    return pkt;
}


int Socket::sendPacket(Packet pkt){
    
    int n = write(this->socketfd, &pkt, sizeof(pkt)); 

    if (n < 0) {
        std::cout << "ERROR writing to socket: " << this->socketfd << std::endl ;
        return n;
    }
    return n;
}



ServerSocket::ServerSocket() : Socket(){
    
    this->serv_addr.sin_family = AF_INET;
	this->serv_addr.sin_port = htons(PORT);
	this->serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(this->serv_addr.sin_zero), 8);
    
}


void ServerSocket::bindAndListen(){
    /*
    if (bind(this->socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		printf("ERROR on binding");
	
	listen(sockfd, MAX_TCP_CONNECTIONS);
	std::cout << "Listening...\n";
    */

}


void ClientSocket::connectToServer(){
    struct sockaddr_in serv_addr;
    struct hostent *server;
	server = gethostbyname(SERVER_ADDR);

    serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     
	

	if (connect(this->getSocketfd(),(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("ERROR connecting\n");
        exit(1);
    }
        
}