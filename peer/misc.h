#include "./peer_global.h"

#ifndef _PMISC
#define _PMISC

#define BUFFER_SIZE 2048


int connectPeer(int * clientSocket, int port);
void createServerSocket(int * serverSocket, int port);
void * receiveDataFunc(void * arg);
void * listenFunc(void * arg);

#endif