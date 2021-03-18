#include "../include/misc.hpp"


// RESOLVI IGNORAR A SUGESTÃO DE USAR UINT16_T PARA TIMESTAMP
// PORQUE A QUANTIDADE DE BYTES A MAIS É POUCA
uint16_t getU16Date() {
    time_t ti;
    ti = time(NULL);
    struct tm tm_time;
    tm_time = *localtime(&ti);

    //const time_t create_time;
    uint16_t d;
    d = tm_time.tm_mday
        + (tm_time.tm_mon + 1) * 32
        + (tm_time.tm_year - (1980-1900)) * 512;

    return d;
}

std::string getStringDateFromU16(uint16_t d){

    char buff[50];
    snprintf(buff, sizeof(buff), "%02d/%02d/%02d", (int) d%32, (int) (d/32)%16, (int) ((d/512)%128 + (1980-1900))%100);
    std::string stringTime = buff;
    return stringTime;
}