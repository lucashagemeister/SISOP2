#include "../include/Server.hpp"

int main(){

	map<string, int> possibleServerAddresses;

	// #####################################################################
	// LER ESSAS INFORMAÇÕES DE UM ARQUIVO DE CONFIGURAÇÃO
	possibleServerAddresses.insert(pair<string, int>(SERVER_ADDR1, PORT));
	possibleServerAddresses.insert(pair<string, int>(SERVER_ADDR2, PORT1));
	possibleServerAddresses.insert(pair<string, int>(SERVER_ADDR3, PORT2));
	possibleServerAddresses.insert(pair<string, int>(SERVER_ADDR4, PORT3));
	// #####################################################################

	ServerSocket serverSocket = ServerSocket();
	Server* server = new Server(possibleServerAddresses);


    pthread_t threadConnections[MAX_TCP_CONNECTIONS];
	pthread_t electionMonitorThread;

	int i = 0;

	serverSocket.bindAndListen(server);

	// Initialize election thread
	group_communiction_handler_args *args = (group_communiction_handler_args *) calloc(1, sizeof(group_communiction_handler_args));
	args->server = server;
	pthread_create(&electionMonitorThread, NULL, Server::electionTimeoutHandler, (void *)args);

	// Try to connect to other servers and defines itself as backup or primary
	serverSocket.connectToGroupMembers(server);
	
	

	while (1){
		serverSocket.connectNewClientOrServer(&threadConnections[i], server);
		i++;
	}
	
	pthread_join(electionMonitorThread, NULL);
	return 0; 
}