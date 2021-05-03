#pragma once
#include <stdint.h>
#include <iostream>
#include <pthread.h>
#include <string>
#include <list>
#include <vector>
#include <map>
#include "Socket.hpp"


class ClientSocket : public Socket {
	public:
		bool connectToServer();
		bool connectToServer(const char* serverAddress, int serverPort);
};


class Client{
public:
    
    string user;
    int originalClientPort;  // Required to inform in a reconnection which port it was firstly running, 
                             // which is an information used to control sessions.
    map<string, int> possibleServerAddresses;   // <ip, port>
    ClientSocket socket;
    
    Client(string user, map<string, int> possibleServerAddresses);
    static void *do_threadSender(void* arg);
	static void *do_threadReceiver(void* arg);
    static void *controlThread(void* arg);

    pthread_mutex_t mutex_print;
    pthread_mutex_t mutex_input;
    pthread_mutex_t mutex_control;
    
 private:
	void cleanBuffer(void);
	void executeSendCommand();
	void executeFollowCommand();
    void establishConnection();
    void reestablishConnection();
    void connectToPrimaryServer(bool reestablishingConnection);

};


