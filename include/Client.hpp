#pragma once
#include <stdint.h>
#include <iostream>
#include <pthread.h>
#include <string>
#include <list>
#include <future>
#include <chrono>
#include "Socket.hpp"


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


