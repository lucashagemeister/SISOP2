#include "../include/Client.hpp"
#include "../include/Notification.hpp"
#include "../include/defines.hpp"
#include <list>

using namespace std;

Client::Client(){

    // stabilishing connection 


}

Client::Client(char *clientName, char *listOfFollowers, int numberOfAccess){
	strcpy(this->clientName, clientName);
	strcpy(this->listOfFollowers, listOfFollowers);
	this->numberOfAccess = numberOfAccess;
}

void Client::do_threadSender(void* arg){
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    list<string> message; //mensagem vai ser uma lista de linhas
    string line;
    char c;
    int characters = 0;
	
    while (TRUE) {
        pthread_mutex_lock(&mutex);
        //INICIO DA SECAO CRITICA
	do {
	   c = getchar();
            if (c != CR) {
                line = line + c;
            }
            else { 
                message.push_back(line);
                line = "";
                cout << endl;
            }
            characters++;

       } while (c != '@' && characters <= MAX_NOTIFICATION_LENGTH+1);
       if (c == '@')
        message.push_back(line);	//pegar última linha da mensagem
	
       cout << endl << "Mensagem enviada:" << endl;
       for (auto v : message)
            std::cout << v << "\n";
        //FIM DA SECAOO CRITICA
        pthread_mutex_unlock(&mutex);
    }
}

void Client::do_threadReceiver(void* arg){

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    Notification* apiTransmission;

    while (TRUE) {
        pthread_mutex_lock(&mutex);
        //INICIO DA SECAO CRITICA
        //apiTransmission = classColombelli.getNewNotification();
        if (apiTransmission != NULL) {
            cout << "Tweet from" << apiTransmission->getAuthor() << "at" << apiTransmission->getTimestamp() << ":" << endl;
            //cout << "%s", apiTransmission._string << endl;
            apiTransmission = NULL;
        }
        //FIM DA SECAO CRITICA
        pthread_mutex_unlock(&mutex);
    }
}

