#include "../include/Server.hpp"
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
using namespace std;


pthread_mutex_t start_session_mutex = PTHREAD_MUTEX_INITIALIZER;

map<string, sem_t> user_sessions_semaphore;
map< string, list< pair<string, int> > > sessions; // {user, [<ip, port>]}


Server::Server(){
}

Server::Server(host_address address){
	this->ip = address.ipv4;
	this->port = address.port;
}



bool Server::try_to_start_session(string user, host_address address) {
    pthread_mutex_lock(&start_session_mutex); // Início SC

    if(user_exists(user)) {
        sem_t num_sessions;
        sem_init(&num_sessions, 0, 2);
        user_sessions_semaphore.insert({user, num_sessions}); // Usuário é criado com a disponibilidade de 2 sessões
    } 
    int session_started = sem_trywait(&(user_sessions_semaphore[user])); // tenta consumir um recurso de sessão
    if(session_started == 0) { 
        // add user and (ip, port) to sessions map if session started
        // needs mutex
        // sessions.insert({user, {ip, port}})
    }
    pthread_mutex_unlock(&start_session_mutex); // Fim SC

    return session_started == 0; // 0 se a sessão foi iniciada, -1 se não
}

bool user_exists(string user) {
    return user_sessions_semaphore.find(user) == user_sessions_semaphore.end();
}

void Server::send(notification notification) {

    for(string user : notification.pending) {
        for(auto host_address : sessions[user]) {
            // do notification send
        }
    }
}

void Server::close_session(string user, host_address address) {

    // remove address from sessions map and signal semaphore
    // sem_post(sem_t *sem);
}