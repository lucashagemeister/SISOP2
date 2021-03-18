#include "../include/Packet.hpp"


Packet::Packet(uint16_t type, char *payload){
    this->type = type;
    this->seqn = 0; //TODO: WHAT IS THIS AGAIN???
    this->length = strlen(payload);
    this->timestamp = time(NULL);
    strcpy(this->payload, payload);
}


Packet::Packet(uint16_t type, time_t timestamp, char *payload){
    this->type = type;
    this->seqn = 0; //TODO: WHAT IS THIS AGAIN???
    this->length = strlen(payload);
    this->timestamp = timestamp;
    strcpy(this->payload, payload);
}

Packet::Packet(packet packet){
    this->type = packet.type;
    this->seqn = packet.seqn;
    this->length = packet.length;
    this->timestamp = packet.timestamp;
    strcpy(this->payload, *packet.payload);
}


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