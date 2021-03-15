#ifndef PACKET_HEADER
#define PACKET_HEADER

#include <stdint.h>
#include <string.h>
#include <string>
#include <ctime>
#include "defines.hpp"


class Packet {

    private:
        uint16_t type;                  // See possible types in defines.hpp
        uint16_t seqn;                  // Sequence number (weverton: "número de sequência do comando para vcs não lidarem com comandos repetidos"??)
        uint16_t length;                // Payload length
        time_t timestamp;               // Data timestamp
        char payload[MAX_PAYLOAD_LENGTH];   // Content of the packet

	public:

        Packet(uint16_t type, char *payload);   // Get the timestamp when initializing the object
        Packet(uint16_t type, time_t timestamp, char *payload);   // Get the provided timestamp

		uint16_t getPacketType();
		uint16_t getPacketSeqn();
		uint16_t getPacketLength();
		uint16_t getPacketTimestamp();
        std::string getPacketPayload();
};


#endif