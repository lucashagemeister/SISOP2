#include "../include/Client.hpp"

int main(int argc, char **argv) {
  Client *client = (Client *) calloc(1, sizeof(Client));
  pthread_t threadSender;
  pthread_t threadReceiver;
  pthread_t threadControl;


  if (argc < 3) {
    fprintf(stderr,"ERROR too few arguments, provide <user> <server_address> and <server_port>\n");
    exit(0);
  }

  string user = argv[1];
  string serverAddress = argv[2];
  int serverPort = atoi(argv[3]);

  client = new Client(user, serverPort, serverAddress);


  pthread_create(&threadControl, NULL, Client::controlThread, (void *)client);
  pthread_create(&threadReceiver, NULL, Client::do_threadReceiver, (void *)client);
  pthread_create(&threadSender, NULL, Client::do_threadSender, (void *)client);

  pthread_join(threadControl, NULL);
  pthread_join(threadReceiver, NULL);
  pthread_join(threadSender, NULL);
    
  return 0;
}
