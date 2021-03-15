#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fstream>
#include <time.h>
#include "comandosCliente.h"
#include <iostream>
using namespace std;


void* clientCommand(void* arg) {
	/*
	char* clientText;
	fgets(clientText,1000,stdin); //1000 eu botei só porque precisar ter o parâmetro tamanho.

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
	*/
	return nullptr;
}





/*
void* clientConnection() {

}


void* clientNotify(void* arg) {
	return nullptr;
}


void* clientFollowing() {

}

void* clientFollowers() {

}
*/