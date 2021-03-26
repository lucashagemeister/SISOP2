#pragma once
#include <stdint.h>
#include <iostream>
#include <pthread.h>
#include <string>
#include "Socket.hpp"

#define MAX_MESSAGE_SIZE 128
#define FALSE 0
#define TRUE  1


class Client{
public:
    
    string user;
    int serverPort;
    string serverAddress;
    ClientSocket socket;

    Client(string user, int serverPort, string serverAddress);
    static void *do_threadSender(void* arg);
	static void *do_threadReceiver(void* arg);
    void startThreads();

 private:
	void cleanBuffer(void);
	void executeSendCommand();
	void executeFollowCommand();
    void establishConnection();

};


