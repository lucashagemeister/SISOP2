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



void ServerSocket::connectNewClient(pthread_t *threadID, void *(*communicationHandler)(void*), Server server){

    int *newsockfd = (int *) calloc(1, sizeof(int));
	socklen_t clilen;
	struct sockaddr_in cli_addr;
    host_address client_address;
    string user;
    bool sessionAvailable;
    

    // Accepting connection to start communicating
    clilen = sizeof(struct sockaddr_in);
    if ((*newsockfd = accept(this->getSocketfd(), (struct sockaddr *) &cli_addr, &clilen)) == -1) {
        std::cout << "ERROR on accepting client connection" << std::endl;
        //exit(1);
        return;
    }

    std::cout << "New connection estabilished on socket: " << *newsockfd << "\n\n";
    Socket newClientSocket = Socket(*newsockfd);
    
    // Verify if there are free sessions available
    // read client username from socket in 'user' var
    Packet *userPacket = newClientSocket.readPacket();
    if (userPacket == NULL){
        std::cout << "Unable to read user information. Closing connection." << std::endl;
        return;     // destructor automatically closes the socket
    } else 
        user = userPacket->getPayload();
    

    client_address.ipv4 = inet_ntoa(cli_addr.sin_addr);
    client_address.port = ntohs(cli_addr.sin_port);
    sessionAvailable = server.try_to_start_session(user, client_address);
    
    if (!sessionAvailable){
        Packet reportTooManySessionPkt = Packet(MESSAGE_PKT, "Unable to connect to server: no sessions available.");
        newClientSocket.sendPacket(reportTooManySessionPkt);
        return; // destructor automatically closes the socket
    }

    // Build args
    communiction_handler_args *args = (communiction_handler_args *) calloc(1, sizeof(communiction_handler_args));
    args->client_address = client_address;
    args->connectedSocket = *newsockfd;
    args->user = user;

    pthread_create(threadID, NULL, communicationHandler, (void *)args);
}