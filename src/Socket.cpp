#include "../include/Socket.hpp"

Socket::Socket(int sourceSocket, int destSocket){
    this->sourceSocket = sourceSocket;
    this->destSocket = destSocket;
}

Packet* Socket::readPacket(bool* connectedClient){

}

int Socket::sendPacket(Packet packet){

}