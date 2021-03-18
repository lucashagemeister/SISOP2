#ifndef PACKET_HEADER
#define PACKET_HEADER

#include <iostream>
#include <stdint.h>
#include <string.h>
#include <string>
#include "misc.hpp"
#include "defines.hpp"


class Packet {

    private:
        uint16_t type;      // See possible types in defines.hpp
        uint16_t seqn;      // Sequence number
        uint16_t length;    // Payload length
        uint16_t timestamp;   // Data timestamp
        char payload[MAX_PAYLOAD_LENGTH];      // Content of the packet

	public:

        Packet(uint16_t type, char *payload);   // Get the timestamp when initializing the object
        Packet(uint16_t type, uint16_t timestamp, char *payload);   
        Packet(uint16_t type, uint16_t seqn, uint16_t timestamp);
        //Packet(packet packet);

		uint16_t getType();
		uint16_t getSeqn();
		uint16_t getLength();
		uint16_t getTimestamp();
        char* getPayload();

        void setType(uint16_t type);
        void setSeqn(uint16_t seqn);
        void setTimestamp(uint16_t timestamp);
        void setPayload(char* payload);
};


#endif