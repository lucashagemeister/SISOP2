#include "../include/Server.hpp"

int main(){

	map<string, int> possibleServerAddresses;
	
	// INÍCIO DA LEITURA DAS INFORMAÇÕES DE UM ARQUIVO DE CONFIGURAÇÃO
	string filename("ipporta.txt");
	ifstream input_file(filename);
	
	string a, b;
	while (input_file >> a >> b){
		possibleServerAddresses.insert(pair<string, string>(a, b));
	}
	
	input_file.close();
	// FIM DA LEITURA DAS INFORMAÇÕES DE UM ARQUIVO DE CONFIGURAÇÃO
	
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
