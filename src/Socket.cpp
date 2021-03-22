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
        exit(1);
    }
    return pkt;
}


int Socket::sendPacket(Packet pkt){
    
    int n = write(this->socketfd, &pkt, sizeof(pkt)); 

    if (n < 0) {
        std::cout << "ERROR writing to socket: " << this->socketfd << std::endl ;
        exit(1);
    }
    return n;
}



ServerSocket::ServerSocket() : Socket(){
    
    this->serv_addr.sin_family = AF_INET;
	this->serv_addr.sin_port = htons(PORT);
	this->serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(this->serv_addr.sin_zero), 8);

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


void ServerSocket::bindAndListen(){
    
    if (bind(this->getSocketfd(), (struct sockaddr *) &this->serv_addr, sizeof(this->serv_addr)) < 0) {
		printf("ERROR on binding");
        exit(1);
    }
	
	listen(this->getSocketfd(), MAX_TCP_CONNECTIONS);
	std::cout << "Listening...\n";
}



void ServerSocket::connectNewClient(pthread_t *threadID, void *(*communicationHandler)(void*)){

    int *newsockfd = (int *) calloc(1, sizeof(int));
	socklen_t clilen;
	struct sockaddr_in cli_addr;
    

    // Accepting connection to start communicating
    clilen = sizeof(struct sockaddr_in);
    if ((*newsockfd = accept(this->getSocketfd(), (struct sockaddr *) &cli_addr, &clilen)) == -1) {
        printf("ERROR on accept");
        exit(1);
    }

    std::cout << "TRYING TO CONNECT IN: " << *newsockfd << "\n\n";

    // Verify if there are free sessions available
    // TO-DO

    // Build args
    communiction_handler_args *args = (communiction_handler_args *) calloc(1, sizeof(communiction_handler_args));
    args->cli_addr = cli_addr;
    args->connectedSocket = *newsockfd;

    pthread_create(threadID, NULL, communicationHandler, (void *)args);
}