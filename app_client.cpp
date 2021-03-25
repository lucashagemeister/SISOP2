#include "./include/Client.hpp"
#include <pthread.h>  


int main() {

    Client *client = (Client *) calloc(1, sizeof(Client));
    pthread_t threadSender;
    pthread_t threadReceiver;

    client = new Client();

    pthread_create(&threadSender, NULL, Client::do_threadSender, (void *)client);
    pthread_create(&threadReceiver, NULL, Client::do_threadReceiver, (void *)client);
    
    return 0;
}
