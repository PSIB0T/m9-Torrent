#ifndef _PGLOBALS
#define _PGLOBALS

#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <map>
#include <vector>
#include <string>
#include <queue>
#include "./../global_commands.h"


#define BUFFER_SIZE 4096
#define CHUNK_SIZE 1024
#define COMMAND_SIZE 100
#define MAX_BACKLOG_SIZE 20000
#define THREAD_POOL_SIZE 30

struct RequestType {
    std::string message;
    int clientSocket;
    int messageSize;
    RequestType();

    RequestType(std::string message, int clientSocket, int messageSize);

};


extern int tracker_socket;
extern int peer_socket;
extern int tracker_port;
extern int peer_port;
extern char address[20];


extern pthread_mutex_t lock;
extern pthread_mutex_t requestLock;
extern pthread_cond_t requestConditionVar;

extern pthread_mutex_t FileDownloadMutex;
extern sem_t FileDownloadSemaphore;
extern int FileDownloadCount;
extern std::map <std::string, pthread_mutex_t> FileDownloadLockMap;

void writerLock(sem_t * w);
void writerUnlock(sem_t * w);
void readerLock(int * numreader, sem_t * writer, pthread_mutex_t * mutex);
void readerUnlock(int * numreader, sem_t * writer, pthread_mutex_t * mutex);


extern std::map <int, int> session;
extern std::map <std::string, std::string> FileMap;
extern pthread_t request_thread_pool[THREAD_POOL_SIZE];
extern std::queue<RequestType *> requestQueue;

#endif
