#ifndef PACKET_HEADER
#define PACKET_HEADER

#include <stdint.h>
#include <string.h>
#include <string>
#include <ctime>
#include "defines.hpp"


// struct to serialize over the transmission
typedef struct packet{
    uint16_t type;
    uint16_t seqn;
    uint16_t length;
    uint16_t timestamp;
    const char* payload[MAX_PAYLOAD_LENGTH];
} packet;


class Packet {

    private:
        uint16_t type;      // See possible types in defines.hpp
        uint16_t seqn;      // Sequence number
        uint16_t length;    // Payload length
        time_t timestamp;   // Data timestamp
        char payload[MAX_PAYLOAD_LENGTH];      // Content of the packet

	public:

        Packet(uint16_t type, char *payload);   // Get the timestamp when initializing the object
        Packet(uint16_t type, time_t timestamp, char *payload);   // Get the provided timestamp
        Packet(packet packet);

		uint16_t getType();
		uint16_t getSeqn();
		uint16_t getLength();
		uint16_t getTimestamp();
        char* getPayload();
};





#endif