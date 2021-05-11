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
        //std::cout << "ERROR reading from socket: " << this->socketfd  << std::endl;
        return NULL;
    }
    else if(n == 0){
        std::cout << "Connection closed." << std::endl;
        return NULL;
    }

    return pkt;
}


Packet* Socket::readPacket(int socketfd){

    Packet* pkt = new Packet();
    memset(pkt, 0, sizeof (Packet));
    int n = read(socketfd, pkt, sizeof(Packet));
    if (n<0){
        std::cout << "ERROR reading from socket: " << socketfd  << std::endl;
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


void Socket::reopenSocket(){
    close(this->socketfd);
    if ((this->socketfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        std::cout << "ERROR reopening socket\n" << std::endl;
        exit(1);
    }
}
