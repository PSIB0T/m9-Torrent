#include "./tracker_global.h"

#ifndef _TMISC
#define _TMISC

#define BUFFER_SIZE 2048

void createServerSocket(int * serverSocket, int port);
void * receiveDataFunc(void * arg);
void * listenFunc(void * arg);

#endif
