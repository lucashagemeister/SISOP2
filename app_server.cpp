#include "./include/Server.hpp"
#include "./include/Socket.hpp"


ServerSocket socket = ServerSocket();
Server server = Server();



void *communicationHandler(void *handlerArgs){

    pthread_t readCommandsT;
    pthread_t sendNotificationsT;

    pthread_create(&readCommandsT, NULL, readCommandsHandler, handlerArgs);
    pthread_create(&sendNotificationsT, NULL, sendNotificationsHandler, handlerArgs);
    return;
}


void *readCommandsHandler(void *handlerArgs){
	struct communiction_handler_args *args = (struct communiction_handler_args *)handlerArgs;
    Socket connectedSocket = Socket(args->connectedSocket);

    while(1){

        Packet* receivedPacket = connectedSocket.readPacket();
        if (receivedPacket == NULL){  // connection closed
            server.close_session(args->user, args->client_address);
            return;
        }

        switch(receivedPacket->getType()){

            case COMMAND_FOLLOW_PKT:
                server.follow_user(args->user, receivedPacket->getPayload());
                break;

            case COMMAND_SEND_PKT:
                server.create_notification(args->user, receivedPacket->getPayload(), receivedPacket->getTimestamp());
                break;

            default:
                break;
        }
    }
}


void *sendNotificationsHandler(void *handlerArgs){
    struct communiction_handler_args *args = (struct communiction_handler_args *)handlerArgs;
    Socket connectedSocket = Socket(args->connectedSocket);
    bool connectionAlive;

    server.retrieve_notifications_from_offline_period(args->user, args->client_address);

    while(1) {
        connectionAlive = server.read_notifications(args->client_address, connectedSocket);
        if (!connectionAlive){
            return;
        }
    }
}


int main(){

    pthread_t threadConnections[MAX_TCP_CONNECTIONS];
	int i = 0;

	socket.bindAndListen();
	while (1){
		socket.connectNewClient(&threadConnections[i], communicationHandler, server);
		i++;
	}

	return 0; 
}