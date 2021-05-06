#include "../include/Packet.hpp"

Packet::Packet(){
    
}

Packet::Packet(uint16_t type, char const *payload){

    uint16_t payloadLength = strlen(payload);
    if (payloadLength > MAX_PAYLOAD_LENGTH) {
        std::cout << "ERROR payload exceeded maximum length\n" << payload << std::endl;
        exit(1);
    } 

    this->type = type;
    this->seqn = 0; 
    this->length = payloadLength;
    this->timestamp = time(NULL);
    this->e = event();
    strcpy(this->payload, payload);
}


Packet::Packet(uint16_t type, time_t timestamp, char const *payload){

    uint16_t payloadLength = strlen(payload);
    if (payloadLength > MAX_PAYLOAD_LENGTH) {
        std::cout << "ERROR payload exceeded maximum length\n" << payload << std::endl;
        exit(1);
    } 

    this->type = type;
    this->seqn = 0;
    this->length = payloadLength;
    this->timestamp = timestamp;
    strcpy(this->payload, payload);
}


Packet::Packet(uint16_t type, time_t timestamp, char const *payload, char const *author){

    uint16_t payloadLength = strlen(payload);
    if (payloadLength > MAX_PAYLOAD_LENGTH) {
        std::cout << "ERROR payload exceeded maximum length\n" << payload << std::endl;
        exit(1);
    } 

    uint16_t authorLength = strlen(author);
    if (authorLength > MAX_AUTHOR_LENGTH) {
        std::cout << "ERROR author exceeded maximum length\n" << author << std::endl;
        exit(1);
    } 

    this->type = type;
    this->seqn = 0;
    this->length = payloadLength;
    this->timestamp = timestamp;
    strcpy(this->payload, payload);
    strcpy(this->author, author);
}


Packet::Packet(uint16_t type, event e){
    this->type = type;
    this->e = e;
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
time_t Packet::getTimestamp(){
    return this->timestamp;
}
char* Packet::getPayload(){
    return this->payload;
}
char* Packet::getAuthor(){
    return this->author;
}


void Packet::setType(uint16_t type){
    this->type = type;
}
void Packet::setSeqn(uint16_t seqn){
    this->seqn = seqn;
}
void Packet::setTimestamp(time_t timestamp){
    this->timestamp = timestamp;
}
void Packet::setPayload(char* const payload){

    uint16_t payloadLength = strlen(payload);
    if (payloadLength > MAX_PAYLOAD_LENGTH) {
        std::cout << "ERROR payload exceeded maximum length\n" << payload << std::endl;
        exit(1);
    } 

    strcpy(this->payload, payload);
    this->length = payloadLength;
}
void Packet::setAuthor(char* const author){

    uint16_t authorLength = strlen(author);
    if (authorLength > MAX_AUTHOR_LENGTH) {
        std::cout << "ERROR author exceeded maximum length\n" << payload << std::endl;
        exit(1);
    } 

    strcpy(this->author, author);
}