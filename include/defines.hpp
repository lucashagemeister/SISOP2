#ifndef PACKET_TYPES
#define PACKET_TYPES
enum{
    NOTIFICATION_PKT,       // Notification sent by someone the user is following
    MESSAGE_PKT,            // To communicate command errors and similar stuff 
    COMMAND_FOLLOW_PKT,     // User wants to follow someone
    COMMAND_SEND_PKT        // User wants to send a notification
};
#endif

#ifndef PAYLOAD_LENGTH
#define PAYLOAD_LENGTH 129      // 128 + '\0' character
#endif