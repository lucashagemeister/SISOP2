#ifndef PACKET_TYPES
#define PACKET_TYPES
enum{
    NOTIFICATION_PKT = 100,     // Notification sent by someone the user is following
    MESSAGE_PKT,                // To communicate command errors and similar stuff 
    COMMAND_FOLLOW_PKT,         // User wants to follow someone
    COMMAND_SEND_PKT,           // User wants to send a notification
    USER_INFO_PKT,              // Sends a username in the payload
    SESSION_OPEN_SUCCEDED,      // When server could connect client to a session
    SESSION_OPEN_FAILED,        // When server could not connect client to a session
    ALREADY_PRIMARY,            // Message to confirm that client is already connected to primary server
    CURRENT_PRIMARY,            // Message containing who's the current primary server
    USER_INFO_RECONNECT,        // Client message to inform the user it has a session opened before primary went down
    ASK_PRIMARY,                // Backup server sends message asking who's the primary server (what's its port)
    PRIMARY_SERVER_PORT,        // Answer of what's the port for the primary server
    SERVER_PEER_CONNECTING,     // Used to inform that the established connection is server-server
    CLIENT_CONNECTING,          //  Used to inform that the established connection is client-server
    
    // For the bully election algorithm
    ELECTION,
    ANSWER,
    COORDINATOR,

};
#endif


#ifndef MAX_PAYLOAD_LENGTH
#define MAX_PAYLOAD_LENGTH 256
#endif

#ifndef MAX_AUTHOR_LENGTH
#define MAX_AUTHOR_LENGTH 18
#endif

#ifndef MAX_NOTIFICATION_LENGTH
#define MAX_NOTIFICATION_LENGTH 128
#endif

#ifndef PKT_HEADER_BUFFER_LENGTH
#define PKT_HEADER_BUFFER_LENGTH 4     
#endif

#ifndef CR
#define CR 13
#endif 

#ifndef LF
#define LF 10
#endif 

#ifndef SERVER_ADDR
#define SERVER_ADDR "127.0.0.1"
#endif

#ifndef MAX_TCP_CONNECTIONS
#define MAX_TCP_CONNECTIONS 256
#endif


#ifndef PORT_BAND
#define PORT_BAND
enum{
    PORT = 4000,    // Primary server in normal situations
    PORT1,          // Backup 1
    PORT2,          // Backup 2
    PORT3           // Backup 3
};
#endif


extern std::vector<int> possiblePorts;