#include "../include/Server.hpp"
#include "../include/Socket.hpp"


ServerSocket serverSocket = ServerSocket();
Server server = Server();


int main(){

    pthread_t threadConnections[MAX_TCP_CONNECTIONS];
	int i = 0;

	serverSocket.bindAndListen();
	while (1){
		serverSocket.connectNewClient(&threadConnections[i], server);
		i++;
	}
	return 0; 
}