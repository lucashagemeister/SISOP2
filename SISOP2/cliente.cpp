#include "Cliente.h"
#include <string.h>
#include <iostream>
using namespace std;

Cliente::Cliente(){

}

Cliente::Cliente(char *clientName, char *listOfFollowers, int numberOfAccess){
	strcpy(this->clientName, clientName);
	strcpy(this->listOfFollowers, listOfFollowers);
	this->numberOfAccess = numberOfAccess;
}

void Cliente::do_threadSender(void* arg){
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    while (TRUE) {
        pthread_mutex_lock(&mutex);
        //INÍCIO DA SEÇÃO CRÍTICA

        //FIM DA SEÇÃO CRÍTICA
        pthread_mutex_unlock(&mutex);
    }
}

void Cliente::do_threadReceiver(void* arg){

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    notification apiTransmission;

    while (TRUE) {
        pthread_mutex_lock(&mutex);
        //INÍCIO DA SEÇÃO CRÍTICA
        apiTransmission = classColombelli.getNewNotification();
        if (apiTransmission != NULL) { //ver direitinho como comparar.
            cout << "Tweet from" << apiTransmission.author << "at" << apiTransmission.timestamp << ":" << endl;
            cout << "%s", apiTransmission._string << endl;
            apiTransmission = NULL;
        }
        //FIM DA SEÇÃO CRÍTICA
        pthread_mutex_unlock(&mutex);
    }
}

/*
void* clientCommand(void* arg) {
	char* clientText;
	fgets(clientText,1000,stdin); //1000 eu botei só porque precisar ter o parâmetro tamanho.

	std::string input = clientText;
	std::string cmd = input.substr(0, input.find(" "));

	if (cmd.compare("CONNECT")) {

	}
	else if(cmd.compare("SEND")) {

	}
	else if (cmd.compare("FOLLOW")) {

	}
	else if (cmd.compare("UNFOLLOW")) {

	}
	else if (cmd.compare("EXIT")) {

	}
	else {
		cout << "Unkown command! Please try again." << endl;
	}
	
return nullptr;
}
*/