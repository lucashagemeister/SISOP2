#include "../include/Client.hpp"
#include "../include/defines.hpp"

using namespace std;

Client::Client(string user, int serverPort, string serverAddress){
    
    this->serverPort = serverPort;
    this->serverAddress = serverAddress;
    this->user = user;
    this->establishConnection();
}

void Client::establishConnection(){

    this->socket.connectToServer(this->serverAddress.c_str(), this->serverPort);
    std::cout << "Connected to server! Trying to send packet with user info..." << "\n\n";
    // Send user information to initiate session
    Packet userInfoPacket = Packet(USER_INFO_PKT, this->user.c_str());
    this->socket.sendPacket(userInfoPacket);

    // Read server answer
    Packet *serverAnswer;
    serverAnswer = this->socket.readPacket();

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


void Client::cleanBuffer(void) {

    char c;
    while ((c = getchar()) != '\n' && c != EOF);

}

std::string skipReceiverMode()
{
   // std::cout << "VOCE TEM 5 SEGUNDOS PARA DAR ENTER E SAIR DO RECEIVER\n";
    std::string inputToSkipReceiverMode;
    std::getline(std::cin, inputToSkipReceiverMode);
    return inputToSkipReceiverMode;
}

void Client::executeSendCommand() {
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

        // Implode list of strings into single string
        string completeNotification; 
        for(const auto &line : message) { 
            completeNotification += line + "\n"; 
        }
        
        int n = this->socket.sendPacket(Packet(COMMAND_SEND_PKT, completeNotification.c_str()));
        if (n<0){
            cout << "Connection lost. Exiting..." << "\n\n";
            exit(1);
        }
}


void Client::executeFollowCommand() {
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

    if (flagSpaces == 0 && person[0] == '@') {
        // Send only the username without @
        this->socket.sendPacket(Packet(COMMAND_FOLLOW_PKT, person.substr(1).c_str()));
    }  
}


void *Client::do_threadSender(void* arg){
    Client *client = (Client*) arg;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    char c; 
    std::string command;
	
    while (true) {  
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
    Packet* readPacket;

    while (true) {
        
        readPacket = client->socket.readPacket();
        if (readPacket == NULL)
            exit(1);  // Connection lost

        
        pthread_mutex_lock(&mutex);
        //INICIO DA SECAO CRITICA

            cout << "Tweet from" << readPacket->getAuthor() << "at" << readPacket->getTimestamp() << ":" << endl;
            cout << readPacket->getPayload() << "\n\n";
            readPacket = NULL;

            std::future<std::string> futureThread = std::async(std::launch::async, skipReceiverMode);
            std::chrono::system_clock::time_point five_seconds_passed = std::chrono::system_clock::now() + std::chrono::seconds(5);
            std::future_status status = futureThread.wait_until(five_seconds_passed);
            
            if (status == std::future_status::ready)
                    auto  result = futureThread.get();
                        //std::cout << " VOCE ESTA NO MODO SENDER \n";    

        //FIM DA SECAO CRITICA
        pthread_mutex_unlock(&mutex); //liberar mutex para sender 
    }
}
