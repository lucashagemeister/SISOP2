#include <string.h>
#include "../include/Packet.hpp"


Packet::Packet(uint16_t type, char *payload){
    this->type = type;
    this->seqn = 0; //TODO: WHAT IS THIS AGAIN???
    this->length = 0; //OF THE PAYLOAD??
    this->timestamp =0; //HOW TO GET THE CURRENT TIME AND CONVERT TO UINT16?
    strcpy(this->payload, payload);
}


Packet::Packet(uint16_t type, uint16_t timestamp, char *payload){
    this->type = type;
    this->seqn = 0; //TODO: WHAT IS THIS AGAIN???
    this->length = 0; //OF THE PAYLOAD??
    this->timestamp = timestamp;
    strcpy(this->payload, payload);
}



