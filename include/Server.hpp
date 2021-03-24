#pragma once
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <list>
#include <string>
#include <map>
#include <vector>
#include <queue>
#include "Socket.hpp"
using namespace std;

typedef struct address {
	string ipv4;
	int port;

    bool operator ==(address other) const {
		return ipv4 == other.ipv4 && port == other.port;
	}

    bool operator <(const address& other) const {
		return port < other.port;
	}
    
} host_address;

typedef struct __notification {

    __notification();
    __notification(uint32_t new_id, time_t new_timestamp, string new_body, uint16_t new_length, uint16_t new_pending) :
        id(new_id), timestamp(new_timestamp), body(new_body), length(new_length), pending(new_pending) {}

    uint32_t id; //Identificador da notificação (sugere-se um identificador único)
    time_t timestamp; //Timestamp da notificação
    string body; //Mensagem
    uint16_t length; //Tamanho da mensagem
    uint16_t pending; //Quantidade de leitores pendentes

    bool operator ==(__notification other) const {
		return id == other.id;
	}

} notification;

class Server
{
public:
    Server();
    Server(host_address addr);
    string ip; 
    int port;

    bool try_to_start_session(string user, host_address address);
    void follow_user(string user, string user_to_follow);
    void create_notification(string user, string body, time_t timestamp);
    void close_session(string user, host_address address);
    void retrieve_notifications_from_offline_period(string user, host_address addr);
    bool read_notifications(host_address addr, Socket socket);


    
private: 
    pthread_mutex_t mutex_session;
    pthread_mutex_t follow_mutex;
    pthread_mutex_t follower_count_mutex;

    pthread_cond_t 	cond_notification_empty, cond_notification_full;
    pthread_mutex_t mutex_notification_sender;

    uint32_t notification_id_counter;

    map<string, sem_t> user_sessions_semaphore;
    map< string, list< host_address > > sessions; // {user, [<ip, port>]}
    map< host_address, priority_queue< uint32_t > > active_users_pending_notifications; // {<ip, port>, [notification]]}
    map< string, list< uint32_t > > users_unread_notifications; // {user, [notification]]}
    map< string, list<string> > followers;
    vector<notification> active_notifications;

    
    bool user_exists(string user);
    bool user_is_active(string user);
    void assign_notification_to_active_sessions(uint32_t notification_id, list<string> followers);

};
