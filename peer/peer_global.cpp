#include "./peer_global.h"


RequestType::RequestType(){}

RequestType::RequestType(std::string message, int clientSocket, int messageSize){
    this->message = message;
    this->clientSocket = clientSocket;
    this->messageSize = messageSize;
}


void writerLock(sem_t * w){
    sem_wait(w);
}

void writerUnlock(sem_t * w){
    sem_post(w);
}

void readerLock(int * numreader, sem_t * writer, pthread_mutex_t * mutex){
    pthread_mutex_lock(mutex);
    *numreader = *numreader + 1;

    if (*numreader == 1){
        sem_wait(writer);
    }

    pthread_mutex_unlock(mutex);
}

void readerUnlock(int * numreader, sem_t * writer, pthread_mutex_t * mutex){
    pthread_mutex_lock(mutex);
    *numreader = *numreader - 1;

    if (*numreader == 0){
        sem_post(writer);
    }

    pthread_mutex_unlock(mutex);
}

int tracker_socket;
int peer_socket;
int tracker_port;
int peer_port;
char address[20] = "0.0.0.0";
bool isLoggedIn = false;


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t requestLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t requestConditionVar = PTHREAD_COND_INITIALIZER;
std::map <int, int> session;
std::map <std::string, std::string> FileMap;
pthread_t request_thread_pool[THREAD_POOL_SIZE];
std::queue<RequestType *> requestQueue;


pthread_mutex_t FileDownloadMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t FileDownloadSemaphore;
int FileDownloadCount;
std::map <std::string, pthread_mutex_t> FileDownloadLockMap;