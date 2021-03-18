#include "../include/Server.hpp"
#include <iostream>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <vector>
using namespace std;



Server::Server()
{

}

Server::Server(host_address address)
{
	this->ip = address.ipv4;
	this->port = address.port;
}

bool Server::try_to_start_session(string user, host_address address)
{
    pthread_mutex_lock(&start_session_mutex); // Início SC

    if(!user_exists(user))
    {
        sem_t num_sessions;
        sem_init(&num_sessions, 0, 2);
        user_sessions_semaphore.insert({user, num_sessions}); // Usuário é criado com a disponibilidade de 2 sessões
        followers.insert(pair<string, vector<string>>(user, vector<string>()));
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

bool Server::user_exists(string user)
{
    return !(user_sessions_semaphore.find(user) != user_sessions_semaphore.end());
}

void Server::create_notification(string user, string body)
{

}

// call this function when new notification is created
void Server::send(notification notification) {
    // go through list of followers and put notification in active followers buffer to be consumed
    // remove notification from active followers list, but keep it on offline followers for future consumption 

    for(string user : notification.pending) {
        for(auto host_address : sessions[user]) {
            // put notification in map of < (ip, port), notifications to send > 
            // when all sessions from same user have notification on its entry, remove @ from list
            // signal consumer that will send to client

            // when consumer wakes, send (?and erase users that have received notification from .pending)
            // when new session is started from user that has notifications to be sent from the period it was offline, wake producer
        }
    }
}
/*                                        

                  |                       |  Lista de notificações recebidas pelo servidor  |  
Lista de perfis   |  Lista de seguidores  |   (id, timestamp, body, length, number of       |  Fila de notificações pendentes de envio aos clientes
                  |                       |  followers pending to receive notification)     |
------------------------------------------------------------------------------------------------------------------------------------------------------
@dez              | @onze, @doze          | 1, 1613618078, "olá", 3, 2                      |
@onze             |                       |                                                 | <@dez 1>
@doze             |                       |                                                 | <@dez 1>   

*/

// call this function when new session is started (after try_to_start_session()) to wake notification producer to client
void Server::send(notification notification) {
    // go through list of notiications pending to be sent to user that have just entered
    // and put them in its entry on < (ip, port), notifications to send > map
    // erase from previous list
    // wake consumer
}

void Server::close_session(string user, host_address address) {

    // remove address from sessions map and signal semaphore and < (ip, port), notification to send > 
    // sem_post(sem_t *sem);
}

void Server::follow_user(string user, string user_to_follow)
{
    followers[user_to_follow].push_back(user);
}