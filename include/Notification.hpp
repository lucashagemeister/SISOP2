#ifndef NOTIFICATION_HEADER
#define NOTIFICATION_HEADER

#include <stdint.h>
#include <string>


class Notification {

    private:
        uint32_t id; //Identificador da notificação (sugere-se um identificador único)
        uint32_t timestamp; //Timestamp da notificação
        uint16_t length; //Tamanho da mensagem
        uint16_t pending; //Quantidade de leitores pendentes

        const std::string message; //Mensagem
        const std::string author;

    public: 
        Notification();

        uint32_t getId();
        uint32_t getTimestamp();
        uint16_t getLength();
        uint16_t getPending();
        std::string getAuthor();
        std::string getMessage();
};

#endif