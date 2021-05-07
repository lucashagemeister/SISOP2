#pragma once

#include <iostream>
#include <stdint.h>
#include <string.h>
#include <string>
#include "defines.hpp"


typedef struct _event 
{
    uint16_t seqn;
    int command; // packet types
    char arg1[MAX_EVENT_ARG1];
    char arg2[MAX_EVENT_ARG2];
    char arg3[MAX_EVENT_ARG3];
    bool committed;

    bool operator ==(_event other) const {
		return seqn == other.seqn && committed == other.committed;
	}
/*        
    _event& operator=(const _event& e) {
        seqn = e.seqn;
        command = e.command;
        arg1 = e.arg1;
        arg2 = e.arg2;
        arg3 = e.arg3;
        bool committed = e.committed;
        
        return *this;
    }
*/
} event;

class Packet {

    private:
        uint16_t type;      // See possible types in defines.hpp
        uint16_t seqn;      // Sequence number
        uint16_t length;    // Payload length
        time_t timestamp;   // Data timestamp
        char author[MAX_AUTHOR_LENGTH];        // If there is one
        char payload[MAX_PAYLOAD_LENGTH];      // Content of the packet

	public:
        event e;
        
        Packet();
        Packet(uint16_t type, char const *payload);   // Get the timestamp when initializing the object
        Packet(uint16_t type, time_t timestamp, char const *payload);   
        Packet(uint16_t type, time_t timestamp, char const *payload, char const *author); // If it's a notification
        Packet(uint16_t type, event e);
        Packet(uint16_t type, event e, uint16_t length);

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
        void setAuthor(char* author);
};

