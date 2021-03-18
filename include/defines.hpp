#ifndef PACKET_TYPES
#define PACKET_TYPES
enum{
    NOTIFICATION_PKT,       // Notification sent by someone the user is following
    MESSAGE_PKT,            // To communicate command errors and similar stuff 
    COMMAND_FOLLOW_PKT,     // User wants to follow someone
    COMMAND_SEND_PKT        // User wants to send a notification
};
#endif

#ifndef MAX_PAYLOAD_LENGTH
#define MAX_PAYLOAD_LENGTH 256
#endif

#ifndef MAX_NOTIFICATION_LENGTH
#define MAX_NOTIFICATION_LENGTH 128
#endif

#ifndef PKT_HEADER_BUFFER_LENGTH
#define PKT_HEADER_BUFFER_LENGTH 4     
#endif