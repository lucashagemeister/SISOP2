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


// returns a pointer to the read Packet object or NULL if connection was closed
Packet* Socket::readPacket(){

    Packet* pkt = new Packet();
    memset(pkt, 0, sizeof (Packet));
    int n = read(this->socketfd, pkt, sizeof(Packet));

    if (n<0){
        std::cout << "ERROR reading from socket: " << this->socketfd  << std::endl;
        return NULL;
    }
    else if(n == 0){
        std::cout << "Connection closed." << std::endl;
        return NULL;
    }

    return pkt;
}


// return the n value gotten from send primitive
int Socket::sendPacket(Packet pkt){
    int n = send(this->socketfd, &pkt, sizeof(pkt), MSG_NOSIGNAL); 
    if (n < 0) 
        std::cout << "ERROR writing to socket: " << this->socketfd << std::endl;
    return n;
}


// overloading for non-initialized Socket object
int Socket::sendPacket(Packet pkt, int socketfd){
    int n = send(socketfd, &pkt, sizeof(pkt), MSG_NOSIGNAL); 
    if (n < 0) {
        std::cout << "ERROR writing to socket: " << this->socketfd << std::endl;
        std::cout << "Connection closed." << std::endl;
    }
    return n;
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

void ClientSocket::connectToServer(string serverAddress, int serverPort){
    struct sockaddr_in serv_addr;
    struct hostent *server;
	server = gethostbyname(serverAddress.c_str());

    serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(serverPort);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     
	

	if (connect(this->getSocketfd(),(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("ERROR establishing connection\n");
        exit(1);
    }
        
}

