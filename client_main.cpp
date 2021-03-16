#include "include/Cliente.hpp"
#include <pthread.h>  //preciso instalar no Windows para fazer funcionar ou ent�o rodar no Linux


int main() {
    pthread_t threadSender;
    pthread_t threadReceiver;

    int n = 10; //ver o que fazer com isso depois

    pthread_create(&threadSender, NULL, do_threadSender, &n);
    pthread_create(&threadReceiver, NULL, do_threadReceiver, &n);

    pthread_join(threadSender, NULL);
    pthread_join(threadReceiver, NULL);

    return 0;

}