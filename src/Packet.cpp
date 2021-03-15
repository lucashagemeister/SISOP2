#include "../include/Packet.hpp"

using namespace std;

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



