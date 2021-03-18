#include "../include/Packet.hpp"


Packet::Packet(uint16_t type, char *payload){
    this->type = type;
    this->seqn = 0; 
    this->length = strlen(payload);
    this->timestamp = getU16Time();
    strcpy(this->payload, payload);
}


Packet::Packet(uint16_t type, uint16_t timestamp, char *payload){
    this->type = type;
    this->seqn = 0;
    this->length = strlen(payload);
    this->timestamp = timestamp;
    strcpy(this->payload, payload);
}


// Useful when it receives a header while reading socket
Packet::Packet(uint16_t type, uint16_t seqn, uint16_t timestamp){
    this->type = type;
    this->seqn = seqn;
    this->timestamp = timestamp;
}

/*
Packet::Packet(packet packet){
    this->type = packet.type;
    this->seqn = packet.seqn;
    this->length = packet.length;
    this->timestamp = packet.timestamp;
    strcpy(this->payload, *packet.payload);
}*/


uint16_t Packet::getType(){
    return this->type;
}
uint16_t Packet::getSeqn(){
    return this->seqn;
}
uint16_t Packet::getLength(){
    return this->length;
}
uint16_t Packet::getTimestamp(){
    return this->timestamp;
}
char* Packet::getPayload(){
    return this->payload;
}