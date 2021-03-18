#include "../include/Socket.hpp"


using namespace std;

Socket::Socket(int socketfd){
    this->socketfd = socketfd;
}

Packet* Socket::readPacket(bool* connectedClient){


    packet receivedPacket;


    // read type
    //read(this->sourceSocket, re)
    // read seqn

    // read length

    // read timestamp

    // read payload



    return NULL;
    /*
	void *receivedPacket = (void*) malloc(sizeof(Packet));
	int receivedBytes = 0;
	int n;


    
	while ( receivedBytes != sizeof(Packet)) {
		n = read(this->sourceSocket, (Packet*)((uintptr_t) receivedPacket + receivedBytes), sizeof(Packet)-receivedBytes);
		if (n < 0) {
			cout << "ERROR reading from socket\n" << endl;
			*connectedClient = false;
			break;
		}
		receivedBytes += n;
	} */

}

int Socket::sendPacket(Packet packet){

    uint16_t type = htons(packet.getType());
    uint16_t seqn = htons(packet.getSeqn());
    uint16_t length = htons(packet.getLength());
    uint16_t timestamp = htons(packet.getTimestamp());
    
    uint16_t buffer[PKT_HEADER_BUFFER_LENGTH] = {type, seqn, length, timestamp};
    int n = write(this->socketfd, buffer, sizeof(buffer)); 

    if (n < 0) {
        cout << "ERROR writing to socket: " << this->socketfd << endl ;
        return n;
    }

    int n = write(this->socketfd, packet.getPayload(), packet.getLength());
    if (n < 0) {
        cout << "ERROR writing to socket: " << this->socketfd << endl ;
    }

    return n;
}