#include "../include/Client.hpp"
#include <pthread.h>  


int main(int argc, char *argv[]) {

    Client *client = (Client *) calloc(1, sizeof(Client));
    pthread_t threadSender;
    pthread_t threadReceiver;

    string user = argv[0];
    string serverAddress = argv[1];
    int serverPort = atoi(argv[2]);

    client = new Client(user, serverPort, serverAddress);

    pthread_create(&threadSender, NULL, Client::do_threadSender, (void *)client);
    pthread_create(&threadReceiver, NULL, Client::do_threadReceiver, (void *)client);
    
    return 0;
}
