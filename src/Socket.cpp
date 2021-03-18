#include "../include/Socket.hpp"


using namespace std;

Socket::Socket(int socketfd){
    this->socketfd = socketfd;
}

Socket::~Socket(){
    close(this->socketfd);
}


Packet* Socket::readPacket(){

    Packet* pkt = new Packet();
    memset(pkt, 0, sizeof (Packet));
    int n = read(this->socketfd, pkt, sizeof(Packet));

    if (n<0){
        cout << "ERROR reading from socket" << endl;
        return NULL;
    }
    return pkt;
}


int Socket::sendPacket(Packet pkt){
    
    int n = write(this->socketfd, &pkt, sizeof(pkt)); 

    if (n < 0) {
        cout << "ERROR writing to socket: " << this->socketfd << endl ;
        return n;
    }
    return n;
}