#include "../include/Server.hpp"

ServerSocket serverSocket = ServerSocket();
Server* server = new Server(ntohs(serverSocket.serv_addr.sin_port));

int main(){

    pthread_t threadConnections[MAX_TCP_CONNECTIONS];
	int i = 0;

	serverSocket.bindAndListen();
	// try to connect to other servers and defines itself as backup or primary
	// depending on if there's any server running (-> backup) or not (-> primary)

	while (1){
		serverSocket.connectNewClient(&threadConnections[i], server);
		i++;
	}
	return 0; 
}