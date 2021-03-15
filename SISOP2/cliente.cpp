#include <pthread.h>
#include "comandosCliente.h"
#define TRUE 1

void do_threadSender(void *arg) {
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    while (TRUE) {
        pthread_mutex_lock(&mutex);
        //INÍCIO DA SEÇÃO CRÍTICA

        //FIM DA SEÇÃO CRÍTICA
        pthread_mutex_unlock(&mutex);
    }
}

void do_threadReceiver(void* arg) {
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    notification apiTransmission;
    while (TRUE) {
        pthread_mutex_lock(&mutex);
        //INÍCIO DA SEÇÃO CRÍTICA
        //apiTransmission = classColombelli.getNewNotification();
        if (apiTransmission != NULL) { //ver direitinho como comparar.
            cout << "Tweet from" << apiTransmission.author << "at" << apiTransmission.timestamp << ":" << endl;
            cout << "%s", apiTransmission._string << endl;
            apiTransmission = NULL;
        }


        //FIM DA SEÇÃO CRÍTICA
        pthread_mutex_unlock(&mutex);
    }
}

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
