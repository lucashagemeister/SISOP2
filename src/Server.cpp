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
    this->notification_id_counter = 0;
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
        users_unread_notifications.insert({user, list<uint32_t>()});
    } 
    
    int session_started = sem_trywait(&(user_sessions_semaphore[user])); // tenta consumir um recurso de sessão
    if(session_started == 0) { 
        // needs mutex
        sessions.insert({user, list<host_address>()});
        sessions[user].push_back(address);
        active_users_pending_notifications.insert({address, list<uint32_t>()});
    }
    pthread_mutex_unlock(&start_session_mutex); // Fim SC

    return session_started == 0; // 0 se a sessão foi iniciada, -1 se não
}

bool Server::user_exists(string user)
{
    return user_sessions_semaphore.find(user) != user_sessions_semaphore.end();
}

void Server::create_notification(string user, string body, time_t timestamp)
{
    pthread_mutex_lock(&follower_count_mutex);

    uint16_t pending_users{0};
    for (auto follower : followers[user])
    {
        pending_notifications[follower].push_back(notification_id_counter);
        pending_users++;
    }

    pthread_mutex_unlock(&follower_count_mutex);

    __notification notif(notification_id_counter, timestamp, body, body.length(), pending_users);
    active_notifications.push_back(notif);
    notification_id_counter += 1;
}

// call this function when new notification is created
void Server::send(uint32_t notification_id, list<string> followers) {
    for(string user : followers) {
        if(user_is_active(user)) {
            for(auto address : sessions[user]) {
                // put notification in map of < (ip, port), notifications to send > 
                active_users_pending_notifications[address].push_back(notification_id);
            }
            // when all sessions from same user have notification on its entry, remove @ from list
            list<uint32_t>::iterator it = find(users_unread_notifications[user].begin(), users_unread_notifications[user].end(), notification_id);
            users_unread_notifications[user].erase(it);
            // signal consumer that will send to client
            // ...
        }
    }
}

bool Server::user_is_active(string user) 
{
    return sessions.find(user) != sessions.end();
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
void Server::retrieve_notifications_from_offline_period(string user, host_address addr) {
    for(auto notif : users_unread_notifications[user]) {
        active_users_pending_notifications[addr].push_back(notif);
    }
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