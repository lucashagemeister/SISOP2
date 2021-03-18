#pragma once
#include <stdint.h>
#include <pthread.h>
#include <list>
#include <string>

typedef struct address {
	string ipv4;
	int port;

    bool operator ==(address other) {
		return ipv4 == other.ipv4 && port == other.port;
	}

} host_address;

class Server
{
public:
    Server();
    Server(host_address addr);
    string ip; 
    int port;
    
private: 
    pthread_mutex_t start_session_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t follower_count_mutex = PTHREAD_MUTEX_INITIALIZER;

    uint32_t notification_id_counter;

    map<string, sem_t> user_sessions_semaphore;
    map< string, list< host_address > > sessions; // {user, [<ip, port>]}
    map< host_address, list< uint32_t > > active_users_pending_notifications; // {<ip, port>, [notification]]}
    map< string, list< uint32_t > > users_unread_notifications; // {user, [notification]]}
    map< string, vector<string> > followers;
    map< string, vector<uint32_t> > pending_notifications;
    vector<__notification> active_notifications;

    bool try_to_start_session(string user, host_address address);
    void send(uint32_t notification_id, list<string> followers);
    void retrieve_notifications_from_offline_period(string user, host_address addr);
    void close_session(string user, host_address address);
    void follow_user(string user, string user_to_follow);
    bool user_exists(string user);
    void create_notification(string user, string body, time_t timestamp);
    bool user_is_active(string user);
    void read_notifications(host_address addr); 
};

typedef struct __notification {

    __notification();
    __notification(uint32_t new_id, time_t new_timestamp, string new_body, uint16_t new_length, uint16_t new_pending) :
        id(new_id), timestamp(new_timestamp), body(new_body), length(new_length), pending(new_pending) {}

    uint32_t id; //Identificador da notificação (sugere-se um identificador único)
    time_t timestamp; //Timestamp da notificação
    string body; //Mensagem
    uint16_t length; //Tamanho da mensagem
    uint16_t pending; //Quantidade de leitores pendentes

    bool operator ==(__notification other) {
		return id == other.id;
	}

} notification;
