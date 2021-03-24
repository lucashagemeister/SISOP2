#include "./Server.hpp"
#include <iostream>
#include <algorithm>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

Server::Server()
{
    mutex_session = PTHREAD_MUTEX_INITIALIZER;
    follow_mutex = PTHREAD_MUTEX_INITIALIZER;
    follower_count_mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_init(&cond_notification_empty, NULL);
    pthread_cond_init(&cond_notification_full, NULL);
    pthread_mutex_init(&mutex_notification_sender, NULL);
}

Server::Server(host_address address)
{
    this->notification_id_counter = 0;
	this->ip = address.ipv4;
	this->port = address.port;

    mutex_session = PTHREAD_MUTEX_INITIALIZER;
    follow_mutex = PTHREAD_MUTEX_INITIALIZER;
    follower_count_mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_init(&cond_notification_empty, NULL);
    pthread_cond_init(&cond_notification_full, NULL);
    pthread_mutex_init(&mutex_notification_sender, NULL);
}

// se não tiver um host_address, eu posso fazer e devolver um session_id
bool Server::try_to_start_session(string user, host_address address)
{
    pthread_mutex_lock(&mutex_session);

    if(!user_exists(user))
    {
        sem_t num_sessions;
        sem_init(&num_sessions, 0, 2);
        user_sessions_semaphore.insert({user, num_sessions}); // user is created with 2 sessions available
        sessions.insert({user, list<host_address>()});
        followers.insert(pair<string, list<string>>(user, list<string>()));
        users_unread_notifications.insert({user, list<uint32_t>()});
    } 
    
    int session_started = sem_trywait(&(user_sessions_semaphore[user])); // try to consume a session resource
    if(session_started == 0) // 0 if session started, -1 if not
    { 
        sessions[user].push_back(address);
        active_users_pending_notifications.insert({address, priority_queue<uint32_t>()});
    }
    pthread_mutex_unlock(&mutex_session); // Fim SC

    return session_started == 0; 
}

bool Server::user_exists(string user)
{
    return user_sessions_semaphore.find(user) != user_sessions_semaphore.end();
}

// call this function when new notification is created
void Server::create_notification(string user, string body, time_t timestamp)
{
    pthread_mutex_lock(&follower_count_mutex);

    uint16_t pending_users{0};
    for (auto follower : followers[user])
    {        
        pthread_mutex_lock(&mutex_notification_sender);
        users_unread_notifications[follower].push_back(notification_id_counter);
        pthread_mutex_unlock(&mutex_notification_sender);

        pending_users++;
    }

    notification notif(notification_id_counter, timestamp, body, body.length(), pending_users);
    active_notifications.push_back(notif);
    assign_notification_to_active_sessions(notification_id_counter, followers[user]);
    notification_id_counter += 1;

    pthread_mutex_unlock(&follower_count_mutex);
}

// call this function after new notification is created
void Server::assign_notification_to_active_sessions(uint32_t notification_id, list<string> followers) 
{
    for (auto user : followers)
    {
        if(user_is_active(user)) 
        {
            pthread_mutex_lock(&mutex_notification_sender);
            for(auto address : sessions[user]) 
            {
                // put notification in map of < (ip, port), notifications to send > 
                active_users_pending_notifications[address].push(notification_id);
            }
            // when all sessions from same user have notification on its entry, remove @ from list
            list<uint32_t>::iterator it = find(users_unread_notifications[user].begin(), users_unread_notifications[user].end(), notification_id);
            users_unread_notifications[user].erase(it);

            // signal consumer that will send to client
            pthread_cond_signal(&cond_notification_full);
            pthread_mutex_lock(&mutex_notification_sender);
        }
    }
}

bool Server::user_is_active(string user) 
{
    return sessions.find(user) != sessions.end();
}

// call this function when new session is started (after try_to_start_session()) to wake notification producer to client
void Server::retrieve_notifications_from_offline_period(string user, host_address addr) 
{
    pthread_mutex_lock(&mutex_notification_sender);
    // go through list of notiications pending to be sent to user that have just entered
    for(auto notification_id : users_unread_notifications[user]) 
    {
        // put them in its entry on < (ip, port), notifications to send > map
        active_users_pending_notifications[addr].push(notification_id);
    }
    // erase from previous list
    (users_unread_notifications[user]).clear();
    // signal consumer
    pthread_cond_signal(&cond_notification_full);
    pthread_mutex_lock(&mutex_notification_sender);
}

// call this function on consumer thread that will feed the user with its notifications
void Server::read_notifications(host_address addr) 
{
    pthread_mutex_lock(&mutex_notification_sender);
    // sleep while user doesn't have notifications to read
    while (active_users_pending_notifications[addr].empty()) { 
        pthread_cond_wait(&cond_notification_full, &mutex_notification_sender); 
    }
    // search active_users_pending_notifications[addr] for all its notification_ids
    while(!active_users_pending_notifications[addr].empty()) 
    {
        uint32_t notification_id = (active_users_pending_notifications[addr]).top();
        for(auto notif : active_notifications)
        {
            if(notif.id == notification_id)
            {
                // socket.send(notif)
                break;
            }
        }
        // erase notifications from active_users_pending_notifications[addr]
        (active_users_pending_notifications[addr]).pop();
    }
    pthread_cond_signal(&cond_notification_empty);
    pthread_mutex_lock(&mutex_notification_sender);
}

// call this function when client presses ctrl+c or ctrl+d
void Server::close_session(string user, host_address address) 
{
    pthread_mutex_lock(&mutex_session);
    // remove address from sessions map and < (ip, port), notification to send > 
    list<host_address>::iterator it = find(sessions[user].begin(), sessions[user].end(), address);
    if(it != sessions[user].end())
    {
        sessions[user].erase(it);
        active_users_pending_notifications.erase(address);

        // signal semaphore
        sem_post(&(user_sessions_semaphore[user]));
    }
    pthread_mutex_unlock(&mutex_session);
}

void Server::follow_user(string user, string user_to_follow)
{
    pthread_mutex_lock(&follow_mutex);

    if (find(followers[user_to_follow].begin(), followers[user_to_follow].end(), user) == followers[user_to_follow].end())
    {
        followers[user_to_follow].push_back(user);
    }

    pthread_mutex_unlock(&follow_mutex);
}