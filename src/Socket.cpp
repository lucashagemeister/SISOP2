#include "../include/Socket.hpp"


using namespace std;

Socket::Socket(int socketfd){
    this->socketfd = socketfd;
}

Socket::~Socket(){
    close(this->socketfd);
}


Packet* Socket::readPacket(){

    // READ HEADER
    // 0-type, 1-seqn, 2-length, 3-timestamp. all of them in "htons"
    uint16_t headerBuffer[PKT_HEADER_BUFFER_LENGTH];
    int n = read(this->socketfd, headerBuffer, sizeof(headerBuffer));

    if (n<0){
        cout << "ERROR reading header from socket" << endl;
        return NULL;
    }

    uint16_t type = ntohs(headerBuffer[0]);
    uint16_t seqn = ntohs(headerBuffer[1]);
    uint16_t length = ntohs(headerBuffer[2]);
    uint16_t timestamp = ntohs(headerBuffer[3]);

    Packet * receivedPacket = new Packet(type, seqn, timestamp);

    // READ PAYLOAD
    char payload[length];
    n = read(this->socketfd, payload, length);
    if (n<0){
        cout << "ERROR reading header from socket" << endl;
        return NULL;
    }

    receivedPacket->setPayload(payload);
    return receivedPacket;
}



int Socket::sendPacket(Packet packet){

    uint16_t type = htons(packet.getType());
    uint16_t seqn = htons(packet.getSeqn());
    uint16_t length = htons(packet.getLength());
    uint16_t timestamp = htons(packet.getTimestamp());
    
    uint16_t buffer[PKT_HEADER_BUFFER_LENGTH] = {type, seqn, length, timestamp};
    int n = write(this->socketfd, buffer, sizeof(buffer)); 

    if (n < 0) {
        cout << "ERROR writing header to socket: " << this->socketfd << endl ;
        return n;
    }

    n = write(this->socketfd, packet.getPayload(), packet.getLength());
    if (n < 0) {
        cout << "ERROR writing payload to socket: " << this->socketfd << endl ;
    }

    return n;
}