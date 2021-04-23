#pragma once
#include <stdint.h>
#include <iostream>
#include <pthread.h>
#include <string>
#include <list>
#include <vector>
#include "Socket.hpp"


class ClientSocket : public Socket {
	public:
		bool connectToServer();
		bool connectToServer(const char* serverAddress, int serverPort);
};


class Client{
public:
    
    string user;
    int serverPort;
    string serverAddress;
    ClientSocket socket;
    
    Client(string user, int serverPort, string serverAddress);
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
    void establishConnection(bool reestablishingConnection);
    void connectToPrimaryServer(bool reestablishingConnection);

};


