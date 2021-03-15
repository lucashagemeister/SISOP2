#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fstream>
#include <time.h>

//vou precisar destes defines dos comandos??
#define CONNECT          0
#define SEND             1
#define FOLLOW           2
#define UNFOLLOW         3
#define EXIT             4
#define MAX_MESSAGE_SIZE 128

typedef struct __packet {
    uint16_t type;              //Tipo do pacote (p.ex. DATA | CMD)
    uint16_t seqn;              //Número de sequência
    uint16_t length;            //Comprimento do payload
    uint16_t timestamp;         // Timestamp do dado
    const char* _payload;       //Dados da mensagem
    uint16_t cmd;               //Comandos Possiveis (será preciso?)
} packet;

typedef struct __notification {
    uint32_t id; //Identificador da notificação (sugere-se um identificador único)
    uint32_t timestamp; //Timestamp da notificação
    const char* _string; //Mensagem
    uint16_t length; //Tamanho da mensagem
    uint16_t pending; //Quantidade de leitores pendentes
    const char* author; //Nome do autor da mensagem (será preciso?)
} notification;

typedef struct _message {
    int cmd;
    char messageContent[MAX_MESSAGE_SIZE];
    int messageSize;
}message;

void* clientCommand(void* arg);
void* clientConnection();
void* clientNotify(void* arg);
void* clientFollowing();
void* clientFollowers();

