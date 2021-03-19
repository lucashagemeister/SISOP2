#include "../include/Client.hpp"
#include "../include/Notification.hpp"
#include <conio.h>
#include <list>
const int MAX_MESSAGE_SIZE = 129; //128 caracteres permitidos + @ de fim de mensagem
const int CR = 13; 
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
        //IN�CIO DA SE��O CR�TICA
	do {
	   c = _getch();
            _putch(c);
            if (c != CR) {
                line = line + c;
            }
            else { 
                message.push_back(line);
                line = "";
                cout << endl;
            }
            characters++;

       } while (c != '@' && characters <=MAX_MESSAGE_SIZE);
       if (c == '@')
        message.push_back(line);	
	
       cout << endl << "Mensagem enviada:" << endl;
        //FIM DA SE��O CR�TICA
        pthread_mutex_unlock(&mutex);
    }

	

    while (TRUE) {
        do {
            
      if (c == '@')
        message.push_back(line); // é preciso disso para pegara última linha da mensagem
       
      cout << endl << "Mensagem enviada: " << endl;
        for (auto v : message)
            std::cout << v << "\n";
    }
}

void Client::do_threadReceiver(void* arg){

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    Notification* apiTransmission;

    while (TRUE) {
        pthread_mutex_lock(&mutex);
        //IN�CIO DA SE��O CR�TICA
        //apiTransmission = classColombelli.getNewNotification();
        if (apiTransmission != NULL) {
            cout << "Tweet from" << apiTransmission->getAuthor() << "at" << apiTransmission->getTimestamp() << ":" << endl;
            //cout << "%s", apiTransmission._string << endl;
            apiTransmission = NULL;
        }
        //FIM DA SE��O CR�TICA
        pthread_mutex_unlock(&mutex);
    }
}

/*
void* clientCommand(void* arg) {
	char* clientText;
	fgets(clientText,1000,stdin); //1000 eu botei s� porque precisar ter o par�metro tamanho.

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
