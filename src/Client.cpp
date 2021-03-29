#include "../include/Client.hpp"
#include "../include/defines.hpp"

using namespace std;



Client::Client(string user, int serverPort, string serverAddress){
    
    this->serverPort = serverPort;
    this->serverAddress = serverAddress;
    this->user = user;
    this->establishConnection();

    pthread_mutex_init(&mutex_print, NULL);
    pthread_mutex_init(&mutex_input, NULL);
    pthread_mutex_init(&mutex_control, NULL);

    pthread_mutex_lock(&mutex_input);
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

void Client::executeSendCommand() {
    list<string> message; //message is a list of lines
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
            line.pop_back(); // remove the '@' sinalizer character from the message
            message.push_back(line);	// get the last message line                
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
    string person = "";
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
        this->socket.sendPacket(Packet(COMMAND_FOLLOW_PKT, person.substr(0, person.size()-1).c_str()));
    }  
}


void *Client::controlThread(void* arg){

    Client *client = (Client*) arg; 

    // How to monitor user input:
    // https://stackoverflow.com/questions/34479795/make-c-not-wait-for-user-input
    fd_set rfds, save_rfds;
    struct timeval tv;
    int retval;
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    save_rfds = rfds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    

    while(true){
        pthread_mutex_lock(&(client->mutex_control));
        rfds = save_rfds;
        retval = select(1, &rfds, NULL, NULL, &tv);
        

        if (retval){   // User hitted Enter key
            pthread_mutex_unlock(&(client->mutex_control));
            pthread_mutex_unlock(&(client->mutex_input));
            sleep(2);  // Waits threadSender get the mutex

            pthread_mutex_lock(&(client->mutex_control)); // Waits sender free mutex to continue monitoring for Enter hits
        }

        pthread_mutex_unlock(&(client->mutex_control));
    }
}



void *Client::do_threadSender(void* arg){
    Client *client = (Client*) arg;
    char c; 
    std::string command;
	
    
    while (true) { 
        pthread_mutex_lock(&(client->mutex_input));
        pthread_mutex_lock(&(client->mutex_control));
        pthread_mutex_lock(&(client->mutex_print));
        client->cleanBuffer();

        command = "";
            //INICIO DA SECAO CRITICA
        cout << "Command mode selected. What would you like to do?" << endl;
        do {
            c = getchar();
            command += c;
        } while (c != LF && c!= ' ');
        command.pop_back(); //remover o LF do final do comando
            
        if (command.compare("FOLLOW") == 0) {
            client->executeFollowCommand();
            cout << "Done!\n";
        }
            
        else if (command.compare("SEND") == 0) {
            client->executeSendCommand();
            cout << "Done!\n";
            client->cleanBuffer();
        }

        else 
            cout << "Command not found! Aborting...\n";

        
        //FIM DA SECAO CRITICA
        pthread_mutex_unlock(&(client->mutex_print));
        pthread_mutex_unlock(&(client->mutex_control));
    }
}


void *Client::do_threadReceiver(void* arg){
    
    Client *client = (Client*) arg; 
    Packet* readPacket;

    while (true) {    
        
        readPacket = client->socket.readPacket();
        if (readPacket == NULL)
            exit(1);  // Connection lost

        pthread_mutex_lock(&(client->mutex_print));
            if (readPacket->getType() == NOTIFICATION_PKT){
                cout << "Tweet from " << readPacket->getAuthor() << " at " << readPacket->getTimestamp() << ":" << endl;
                cout << readPacket->getPayload() << "\n\n";
                readPacket = NULL;
            }
            else if (readPacket->getType() == MESSAGE_PKT){
                cout << "\nMessage from server: " << readPacket->getPayload() << "\n";
            }
        pthread_mutex_unlock(&(client->mutex_print));
    }
}


void ClientSocket::connectToServer(){
    struct sockaddr_in serv_addr;
    struct hostent *server;
	server = gethostbyname(SERVER_ADDR);

    serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(PORT);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);     
	

	if (connect(this->getSocketfd(),(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("ERROR connecting\n");
        exit(1);
    }        
}

void ClientSocket::connectToServer(const char* serverAddress, int serverPort){
    struct sockaddr_in serv_addr;
    struct hostent *server;
	server = gethostbyname(serverAddress);
    

    serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(serverPort);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);
	

	if (connect(this->getSocketfd(),(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("ERROR establishing connection\n");
        exit(1);
    }   
}