#include "./misc.h"


int connectPeer(int * clientSocket, int port){
    *clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in  * server_address = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(port);
    server_address->sin_addr.s_addr = INADDR_ANY;
    int conn_status = connect(*clientSocket, (struct sockaddr *) server_address, sizeof(*server_address));
    return conn_status;
}

void handleRequest(std::string messageString, int clientSocket){
    // std::cout << messageString << std::endl;
    std::string command = messageString.substr(0, messageString.find(":"));
    std::string data = messageString.substr(messageString.find(":") + 1, messageString.size());
    if (command == FileStreamRecv){
        std::string fileName = data;
        downloadFile(fileName, clientSocket);
    } else if (command == RequestFilePeer){
        std::string fileName = data;
        uploadFile(fileName, clientSocket);
    } else {
        std::cout << "Data not recognized!" << std::endl;
    }
    // fflush(stdin);
}

void * handleRequestThread(void * data){
    RequestType * rType;
    while(true){
        pthread_mutex_lock(&requestLock);
        while (requestQueue.empty()){
            pthread_cond_wait(&requestConditionVar, &requestLock);
        }
        rType = requestQueue.front();
        requestQueue.pop();
        pthread_mutex_unlock(&requestLock);
        handleRequest(rType->message, rType->clientSocket);

        delete rType;
    }
    return NULL;
}

// void * handleRequest(void * arg){
//     RequestType * rType = (RequestType *) arg;
//     std::string messageString = rType->message;
//     std::string command = messageString.substr(0, messageString.find(":"));
//     std::string data = messageString.substr(messageString.find(":") + 1, messageString.size());
//     if (command == FileStreamRecv){
//         std::string fileName = data;
//         downloadFile(fileName, rType->clientSocket);
//     } else if (command == RequestFilePeer){
//         std::string fileName = data;
//         uploadFile(fileName, rType->clientSocket);
//     } else {
//         std::cout << "Data not recognized!" << std::endl;
//     }
//     free(arg);
//     return NULL;
// }


void createServerSocket(int * serverSocket, int port){
    *serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in  * server_address = (struct sockaddr_in  *) malloc(sizeof(struct sockaddr_in));
    inet_pton(AF_INET, address, &server_address->sin_addr);
    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(port);
    server_address->sin_addr.s_addr = INADDR_ANY;
    int server_status = bind(*serverSocket, (struct sockaddr *) server_address, sizeof(*server_address));
    printf("%d\n", server_status);
    if (server_status == -1){
        printf("Error allocating port\n");
        exit(1);
    }
}


void disconnectSession(int clientSocket){
    printf("Peer %d disconnected!\n",clientSocket);
    pthread_mutex_lock(&lock);
    session.erase(session.find(clientSocket));
    pthread_mutex_unlock(&lock);
    close(clientSocket);
}

void flushBacklog (std::string &messageBacklog, int clientSocket){
    int sepPosition;
    int dataLen;
    std::string dataLenString;
    std::string finalMessage;
    // std::cout << "In flush backlog initial size is " <<  messageBacklog.size()<< std::endl;
    while(messageBacklog.size() >  0 && (sepPosition = messageBacklog.find(":")) != std::string::npos){
        sepPosition = messageBacklog.find(":");
        dataLenString = messageBacklog.substr(0, sepPosition);
        // std::cout << "DatalenString is " << dataLenString << std::endl;
        // std::cout << "Interim message backlog size is " << messageBacklog.size() << std::endl;
        // std::cout << "Subtract: " << (messageBacklog.size() - dataLen - dataLenString.size()) << std::endl;
        dataLen = stoi(dataLenString);
        if (((int)messageBacklog.size() - dataLen - (int)dataLenString.size()) < 0)
            break;

        finalMessage = messageBacklog.substr(sepPosition + 1, dataLen);
        pthread_mutex_lock(&requestLock);
        requestQueue.push(new RequestType(finalMessage, clientSocket, finalMessage.size()));
        pthread_cond_signal(&requestConditionVar);
        pthread_mutex_unlock(&requestLock);

        messageBacklog.erase(0, dataLenString.size() + dataLen);
    }
//    std::cout << "Final block size is " <<  messageBacklog.size()<< std::endl;
}

void * receiveDataFunc(void * arg){
    int clientSocket = *((int *) arg);
    free(arg);
    char server_data[BUFFER_SIZE] = "";
    pthread_t * requestHandlerThread;
    int status, remainingData;
    std::string messageBacklog = "", finalMessage;
    bool isDisconnectStatus = false;
    int sepPosition;
    // std::cout << "Listening to connections for " << clientSocket << std::endl;
    while(1){
        if (messageBacklog.size() > 0)
            flushBacklog(messageBacklog, clientSocket);
        // std::cout << "Baclog block size is " <<  messageBacklog.size()<< std::endl;            
        status = recv(clientSocket, &server_data, sizeof(server_data), 0);
        
        if (status <= 0){
            disconnectSession(clientSocket);
            break;
        }
        messageBacklog += convertToString(server_data, status);
        sepPosition = messageBacklog.find(":");
        std::string dataLen = messageBacklog.substr(0, sepPosition);
        // std::cout << "In receive func, datalen is " << dataLen << std::endl;
        remainingData = (int)dataLen.size() + (int)stoi(dataLen) - messageBacklog.size();
        while (remainingData > 0){
            // std::cout << "In while loop!" << std::endl;
            bzero(server_data, BUFFER_SIZE);
            status = recv(clientSocket, &server_data, remainingData, 0);
            if (status <= 0){
                disconnectSession(clientSocket);
                isDisconnectStatus = true;
                break;
            }
            messageBacklog += convertToString(server_data, status);
            // std::cout << "Received remaining data " << status << std::endl;
            remainingData -= status;
        }
        // std::cout << "Exited while loop!" << std::endl;
        if (isDisconnectStatus)
            break;
        
        finalMessage = messageBacklog.substr(sepPosition + 1, stoi(dataLen));

        pthread_mutex_lock(&requestLock);
        requestQueue.push(new RequestType(finalMessage, clientSocket, finalMessage.size()));
        pthread_cond_signal(&requestConditionVar);
        pthread_mutex_unlock(&requestLock);

        // handleRequest(messageBacklog.substr(sepPosition + 1, stoi(dataLen)), clientSocket);
        bzero(server_data, BUFFER_SIZE);
        messageBacklog.erase(0, dataLen.size() + stoi(dataLen));
    }
    return NULL;
}


void * listenFunc(void * arg){
    int * peerSocket;
    pthread_t * receiveThread;
    // std::cout << "Listening for connections" << std::endl;
    while(1){
        listen(peer_socket, 100);
        int client_socket = accept(peer_socket, NULL, NULL);
        if (client_socket == -1){
            printf("Error connecting to client!\n");
            continue;
        }
        pthread_mutex_lock(&lock);
        session[client_socket] = 1;
        pthread_mutex_unlock(&lock);

        receiveThread = (pthread_t *)malloc(sizeof(pthread_t));
        // printf("Connected to client %d\n", client_socket);

        peerSocket = (int *)malloc(sizeof(int));
        *peerSocket = client_socket;

        // Listen to peer messages if connection is established.
        pthread_create(receiveThread, NULL, receiveDataFunc, peerSocket);
    }
    return NULL;
}