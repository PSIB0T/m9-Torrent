#include "./peer_global.h"
#include "./fileFunc.h"

#ifndef _PMISC
#define _PMISC

struct RequestType {
    std::string message;
    int clientSocket;
    int messageSize;
    RequestType();

    RequestType(std::string message, int clientSocket, int messageSize);

};

int connectPeer(int * clientSocket, int port);
void createServerSocket(int * serverSocket, int port);
void * receiveDataFunc(void * arg);
void * listenFunc(void * arg);

#endif