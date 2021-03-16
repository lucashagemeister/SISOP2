#pragma once
#include <stdint.h>
#include <list>
#include <string>

class Server{
    public:
        Server();
        Server(host_address addr);
        string ip; 
        int port;
    
    private: 
    bool try_to_start_session(string user, host_address address);
    void send(notification notification);
    void close_session(string user, host_address address);
};

typedef struct __notification {
    uint32_t id; //Identificador da notificação (sugere-se um identificador único)
    uint32_t timestamp; //Timestamp da notificação
    const char* _string; //Mensagem
    uint16_t length; //Tamanho da mensagem
    list<string> pending; //Quantidade de leitores pendentes

} notification;


typedef struct address {
	string ipv4;
	int port;

} host_address;