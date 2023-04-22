#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 1883
#define BACKLOG 5

typedef struct{
    int fd;
    char sUsername[20];
    int iKeepAlive;
    int iKeepAliveMax;
    bool bFashion;//0.-None    1.-Subscribe
    bool bFood;
    bool bMusic;
}r_Client;

typedef struct{
    uint8_t bFrameType;
    uint16_t wMsgLen;
    uint16_t wProtlNameLen;
    char sProtName[4];
    uint8_t bVersion;
    uint8_t bConnectFlags;
    uint16_t bKeepAlive;
    uint16_t wClientIdLen;
    char sClientID [20]; //Aqui va el nombre
}f_Connect;

typedef struct{
    uint8_t bFrameType;    
    uint8_t bRemainLen;
    uint8_t bReservedVal;
    uint8_t bReturnCode;
    /*
    0.- Connection accepted, 
    1.- Unaccepted protocol verison
    2.- Identifier rejected
    3.- Server unavailable
    4.- Bad username or password
    5.- Not authorized
    */
}f_ConnAcknowledge;

typedef struct{
    uint8_t bFrameType;
    uint8_t bkeepAlive;
}f_PingRequest;

typedef struct{
    uint8_t bFrameType;
    uint8_t bresponse;
}f_PingResponse;

typedef struct{
    uint8_t bFrameType;
    uint8_t bRemainLen;
    uint8_t bTopic;
}f_Subscribe;

typedef struct{
    uint8_t bFrameType;
    uint8_t bRemainLen;
    uint8_t bResponse;
}f_SubAcknowledge;

typedef struct{
    uint8_t bFrameType;
    uint8_t bRemainLen;
    uint8_t bTopic;
    char sMsg [50];
}f_Publish;

typedef struct{
    uint8_t bFrameType;
    uint8_t bRemainLen;
}f_PubAcknowledge;
