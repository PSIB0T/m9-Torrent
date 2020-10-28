#include "./tracker_global.h"

#ifndef _TMISC
#define _TMISC


void createServerSocket(int * serverSocket, int port);
void * receiveDataFunc(void * arg);
void * listenFunc(void * arg);

#endif
