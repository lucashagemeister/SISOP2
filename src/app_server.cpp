#include "../include/Server.hpp"

inline bool do_file_exists (const std::string& name) {
    return ( access( name.c_str(), F_OK ) != -1 );
}

int main(){

	map<string, int> possibleServerAddresses;
	
	// INÍCIO DA LEITURA DAS INFORMAÇÕES DE UM ARQUIVO DE CONFIGURAÇÃO
	string filename("../ipporta.txt");
	ifstream input_file(filename);

	if (!do_file_exists(filename)){
		cout << "ERROR config file not found. You should cd to ./bin/ folder!\n";
		exit(1);
	}

	string ip, port;
	while (input_file >> ip >> port){
		possibleServerAddresses.insert(pair<string, int>(ip, atoi(port.c_str())));
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
