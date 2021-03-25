#ifndef PACKET_HEADER
#define PACKET_HEADER

#include <iostream>
#include <stdint.h>
#include <string.h>
#include <string>
#include "defines.hpp"


class Packet {

    private:
        uint16_t type;      // See possible types in defines.hpp
        uint16_t seqn;      // Sequence number
        uint16_t length;    // Payload length
        time_t timestamp;   // Data timestamp
        char author[MAX_AUTHOR_LENGTH];        // If there is one
        char payload[MAX_PAYLOAD_LENGTH];      // Content of the packet

	public:
        Packet();
        Packet(uint16_t type, char const *payload);   // Get the timestamp when initializing the object
        Packet(uint16_t type, time_t timestamp, char const *payload);   
        Packet(uint16_t type, time_t timestamp, char const *payload, char const *author);   
        Packet(uint16_t type, uint16_t seqn, time_t timestamp);

		uint16_t getType();
		uint16_t getSeqn();
		uint16_t getLength();
		time_t getTimestamp();
        char* getPayload();
        char* getAuthor();

        void setType(uint16_t type);
        void setSeqn(uint16_t seqn);
        void setTimestamp(time_t timestamp);
        void setPayload(char* payload);
};


#endif