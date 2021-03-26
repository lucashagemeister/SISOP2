#include "../include/Client.hpp"
#include <pthread.h>  
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char **argv) {
    Client *client = (Client *) calloc(1, sizeof(Client));
    pthread_t threadSender;
    pthread_t threadReceiver;
    
    
    if (argc < 3) {
      fprintf(stderr,"ERROR too few arguments, provide <user> <server_address> and <server_port>\n");
      exit(0);
    }
    
    string user = argv[1];
    string serverAddress = argv[2];
    int serverPort = atoi(argv[3]);

    client = new Client(user, serverPort, serverAddress);

    pthread_create(&threadSender, NULL, Client::do_threadSender, (void *)client);
    pthread_create(&threadReceiver, NULL, Client::do_threadReceiver, (void *)client);
    
    return 0;
}
