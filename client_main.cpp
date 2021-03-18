#include "./include/Cliente.hpp"
#include <pthread.h>  

int main() {
    pthread_t threadSender;
    pthread_t threadReceiver;

    int n = 10; 

    pthread_create(&threadSender, NULL, do_threadSender, &n);
    pthread_create(&threadReceiver, NULL, do_threadReceiver, &n);

    pthread_join(threadSender, NULL);
    pthread_join(threadReceiver, NULL);

    return 0;

}
