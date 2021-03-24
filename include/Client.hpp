#pragma once
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <pthread.h>
#include "Socket.hpp"

#define MAX_MESSAGE_SIZE 128
#define FALSE 0
#define TRUE  1

class Client{
public:
    Client();
    Client(char *clientName, char *listOfFollowers, int numberOfAccess);
    char* clientName; 
    char* listOfFollowers;
    int numberOfAccess;

 private:
	void do_threadSender(void* arg);
	void do_threadReceiver(void* arg);
	void cleanBuffer(void);
	void executeSendCommand();
	void executeFollowCommand();

};

