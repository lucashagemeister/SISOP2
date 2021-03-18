#pragma once
#include <stdint.h>
#include <pthread.h>
#include <list>
#include <string>

typedef struct address {
	string ipv4;
	int port;

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

    map<string, sem_t> user_sessions_semaphore;
    map< string, list< host_address > > sessions; // {user, [<ip, port>]}
    map< host_address, list< notification > > active_users_pending_notifications; // {<ip, port>, [notification]]}
    map< string, list< notification > > users_unread_notifications; // {user, [notification]]}
    map< string, vector<string> > followers;

    bool try_to_start_session(string user, host_address address);
    void send(notification notification, list<string> followers);
    void retrieve_notifications_from_offline_period(string user, host_address addr);
    void close_session(string user, host_address address);
    void follow_user(string user, string user_to_follow);
    bool user_exists(string user);
    void create_notification(string user, string body);
    bool user_is_active(string user);
};

typedef struct __notification {
    uint32_t id; //Identificador da notificação (sugere-se um identificador único)
    uint32_t timestamp; //Timestamp da notificação
    const char* _string; //Mensagem
    uint16_t length; //Tamanho da mensagem
    list<string> pending; //Quantidade de leitores pendentes

} notification;
