#include "../include/Client.hpp"
#include "../include/defines.hpp"
#include <list>

using namespace std;

Client::Client(string user, int serverPort, string serverAddress){

    this->serverPort = serverPort;
    this->serverAdress = serverAdress;
    this->user = user;
    this->establishConnection();
}

void Client::establishConnection(){

    ClientSocket socket = ClientSocket();
    this->socket = socket;
    socket.connectToServer(this->serverAdress, this->serverPort);

    // Send user information to initiate session
    Packet userInfoPacket = Packet(USER_INFO_PKT, this->user.c_str());
    socket.sendPacket(userInfoPacket);

    // Read server answer
    Packet *serverAnswer;
    serverAnswer = socket.readPacket();

    if (serverAnswer != NULL){
        cout << serverAnswer->getPayload() << "\n\n";

        if (serverAnswer->getType() == SESSION_OPEN_SUCCEDED)
            return;
        if (serverAnswer->getType() == SESSION_OPEN_FAILED)
            exit(1);

    } else {
        cout << "Server did not respond, aborting..." << endl;
        exit(1);
    }
}


void cleanBuffer(void) {

    char c;
    while ((c = getchar()) != '\n' && c != EOF);

}

void executeSendCommand() {
    list<string> message; //mensagem vai ser uma lista de linhas
    string line;
    char c;
    int characters = 0;
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

        } while (c != '@' && characters <= MAX_NOTIFICATION_LENGTH + 1);
        if (c == '@') {
            line.pop_back(); //remover o "@" da mensagem, pois ele eh soh um sinalizador de fim de mensagem
            message.push_back(line);	//pegar última linha da mensagem                
        }
        // enviar message (aqui é uma notificação completa)
        //!!! COLOMBELLI, ESTE FOR SÓ IMPRIME MESSAGE PARA VER SE ESTÁ FUNCIONANDO, MAS TEMOS DE TROCAR PARA ALGO QUE FAÇA COM QUE MESSAGE SEJA ENVIADO AO SOCKET. COMO FAZEMOS?
    std::cout << "\nSent Message" << endl;
    for (auto v : message)
        std::cout << v << "\n";
}

void executeFollowCommand() {
    string person;
    char c;
    int characters = 0;
    int flagSpaces = 0;

    do {
        c = getchar();
        if (c != CR) {
            person = person + c;
        }
        if (c == ' ') {
            std::cout << "\nInvalid username! A username does not have whitespaces!" << endl;
            flagSpaces++;
        }
            
    } while (c != LF && characters <= MAX_NOTIFICATION_LENGTH + 1 && c!= ' ');

    if (person[0] != '@') {
        std::cout << "\nInvalid username! A username starts with '@' (@username)." << endl;
    }

    // seguir a person socket.send()
    //!!! COLOMBELLI, ESTE SÓ IMPRIME QUEM A PESSOA QUER SEGUIR PARA VER SE ESTÁ FUNCIONANDO, MAS TEMOS DE TROCAR PARA ALGO QUE FAÇA COM QUE PERSON SEJA ENVIADO AO SOCKET. COMO FAZEMOS?
    if (flagSpaces == 0 && person[0] == '@') {
        std::cout << "Now you're following " << person << endl;
    }  
}


void *Client::do_threadSender(void* arg){
    Client *client = (Client*) arg;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    char c; 
    std::string command;
	
    while (TRUE) {  
        pthread_mutex_lock(&mutex);
	    
        //INICIO DA SECAO CRITICA	    
	do {
            c = getchar();
            command = command + c;
        } while (c != LF && c!= ' ');
        command.pop_back(); //remover o LF do final do comando
	    
	if (command.compare("FOLLOW") == 0) {
            client->executeFollowCommand();
            client->cleanBuffer();
        }
        else if (command.compare("SEND") == 0) {
            client->executeSendCommand();
            client->cleanBuffer();
        }
        else {
            cout << "Command not found! Please try again." << endl;
        }	  	    
        //FIM DA SECAO CRITICA

        pthread_mutex_unlock(&mutex);
    }
}

void *Client::do_threadReceiver(void* arg){

    Client *client = (Client*) arg; 

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    Packet* apiTransmission;

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