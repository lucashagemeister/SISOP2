#pragma once
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <list>
#include <string>
#include <map>
#include <vector>
#include <queue>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include "Socket.hpp"
using namespace std;



typedef struct __notification {

    __notification();
    __notification(uint32_t new_id, string new_author, time_t new_timestamp, string new_body, uint16_t new_length, uint16_t new_pending) :
        id(new_id), author(new_author), timestamp(new_timestamp), body(new_body), length(new_length), pending(new_pending) {}

    uint32_t id; //Identificador da notificação (sugere-se um identificador único)
    string author; 
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
    Server(int port);
    Server(host_address addr);
    string ip; 
    int port;
    vector<int> possibleServerPeerPorts; 
    int portPrimarySever;
    bool backupMode; 

    bool try_to_start_session(string user, host_address address);
    void follow_user(string user, string user_to_follow);
    void create_notification(string user, string body, time_t timestamp);
    void close_session(string user, host_address address);
    void retrieve_notifications_from_offline_period(string user, host_address addr);
    void read_notifications(host_address addr, vector<notification>* notifications);

    void updatePossibleServerPeerPorts();

    static void *groupCommunicationHandler(void *handlerArgs);
    static void *groupReadMessagesHandler(void *handlerArgs);
    static void *groupSendMessagesHandler(void *handlerArgs);


    static void *communicationHandler(void *handlerArgs);
    static void *readCommandsHandler(void *handlerArgs);
    static void *sendNotificationsHandler(void *handlerArgs);


    void print_users_unread_notifications();
    void print_sessions();
    void print_active_notifications();
    void print_active_users_unread_notifications();
    void print_followers();

    
private: 
    pthread_mutex_t mutex_session;
    pthread_mutex_t follow_mutex;
    pthread_mutex_t follower_count_mutex;

    pthread_cond_t 	cond_notification_empty, cond_notification_full;
    pthread_mutex_t mutex_notification_sender;

    uint32_t notification_id_counter;

    map<string, sem_t> user_sessions_semaphore;
    map< string, list< host_address > > sessions; // {user, [<ip, port>]}
    map< string, list< uint32_t > > users_unread_notifications; // {user, [notification]]}
    map< string, list<string> > followers;
    vector<notification> active_notifications;
    map< host_address, priority_queue< uint32_t, vector<uint32_t>, greater<uint32_t> > > active_users_pending_notifications; // {<ip, port>, min_heap[notification]]}

    bool user_exists(string user);
    bool user_is_active(string user);
    void assign_notification_to_active_sessions(uint32_t notification_id, list<string> followers);


};


struct communiction_handler_args {
	Socket* connectedSocket;
	host_address client_address; 
	string user;
    Server* server;
};

struct group_communiction_handler_args {
	sockaddr_in peerServerAddress; 
    Socket* connectedSocket;
    Server* server;
    bool isAcceptingConnection;  // Wether the connection started by this server instance was by accepting (true),
                                 // where the opposite would be it trying to connect to other instance (false)
};

class ServerSocket : public Socket {
	
	public:
		struct sockaddr_in serv_addr;

		void bindAndListen(Server* server);
		void connectNewClientOrServer(pthread_t *threadID, Server *server);
        void connectToGroupMembers(Server* server);
        bool connectToMember(sockaddr_in serv_addr, Server* server);

		ServerSocket();
};