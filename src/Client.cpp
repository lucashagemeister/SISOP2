#include "../include/Client.hpp"
#include "../include/Notification.hpp"

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
    char *ch;
    ch = (char*)malloc(1 * sizeof(char)); //pois é, único jeito que to conseguindo é alocar memória para um caractere apenas. Vou ter que olhar um jeito melhor depois.
	//char **message;
	//message = (char**) malloc(MAX_MESSAGE_SIZE * sizeof(char*));
	//message[0] = (char*)malloc(MAX_MESSAGE_SIZE * sizeof(char));
    int characters = 0;
    char* message;
    message = (char*)malloc(MAX_MESSAGE_SIZE * sizeof(char));
    int i = 0;
    //int j = 0;	
    while (TRUE) {
        pthread_mutex_lock(&mutex);
        //IN�CIO DA SE��O CR�TICA
	do {
		ch[0] = _getch();
		if (ch[0] == CR) {
			//j++;
			cout << endl;
			//message[j] = (char*)malloc(MAX_MESSAGE_SIZE * sizeof(char));
		}
			
		if (characters <= MAX_MESSAGE_SIZE) {
			_putch(ch[0]);
			//message[i][j] = ch;
			//message[i] = ch;
			message[i] = ch[0];
			i++;
			characters++;
		}
		

	} while (ch[0] != '@');
	
	cout << "\nMensagem enviada:" << (char*)message;
        //FIM DA SE��O CR�TICA
        pthread_mutex_unlock(&mutex);
    }
	
	free(message);
	free(ch);
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
