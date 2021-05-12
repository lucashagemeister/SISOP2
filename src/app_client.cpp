#include "../include/Client.hpp"

inline bool do_file_exists (const std::string& name) {
    return ( access( name.c_str(), F_OK ) != -1 );
}


int main(int argc, char **argv) {

  Client *client = (Client *) calloc(1, sizeof(Client));
  pthread_t threadSender;
  pthread_t threadReceiver;
  pthread_t threadControl;


  if (argc < 2) {
    fprintf(stderr,"ERROR you must provide the '@username' argument.\n");
    exit(0);
  }

  string user = argv[1];
  if (user[0] != '@'){
    fprintf(stderr,"ERROR you must use an @ symbol before the username.\n");
    exit(0);
  }

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
	// FIM DA LEITURA DAS INFORMAÇÕES DE UM ARQUIVO DE CONFIGURAÇÃ


  client = new Client(user, possibleServerAddresses);

  pthread_create(&threadControl, NULL, Client::controlThread, (void *)client);
  pthread_create(&threadReceiver, NULL, Client::do_threadReceiver, (void *)client);
  pthread_create(&threadSender, NULL, Client::do_threadSender, (void *)client);

  pthread_join(threadControl, NULL);
  pthread_join(threadReceiver, NULL);
  pthread_join(threadSender, NULL);
    
  return 0;
}
