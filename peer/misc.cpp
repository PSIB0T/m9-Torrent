#include "./misc.h"


RequestType::RequestType(){}

RequestType::RequestType(std::string message, int clientSocket, int messageSize){
    this->message = message;
    this->clientSocket = clientSocket;
    this->messageSize = messageSize;
}

int connectPeer(int * clientSocket, int port){
    *clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in  * server_address = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(port);
    server_address->sin_addr.s_addr = INADDR_ANY;
    int conn_status = connect(*clientSocket, (struct sockaddr *) server_address, sizeof(*server_address));
    return conn_status;
}

void handleRequest(char * message, int clientSocket, int messageSize){
    std::string messageString = convertToString(message, messageSize);
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


void * receiveDataFunc(void * arg){
    int clientSocket = *((int *) arg);
    free(arg);
    char server_data[BUFFER_SIZE] = "";
    pthread_t * requestHandlerThread;
    int status;
    // std::cout << "Listening to connections for " << clientSocket << std::endl;
    while(1){
        status = recv(clientSocket, &server_data, sizeof(server_data), 0);
        std::cout << "Status is " << status << std::endl;
        if (status <= 0){
            printf("Peer %d disconnected!\n",clientSocket);
            pthread_mutex_lock(&lock);
            session.erase(session.find(clientSocket));
            pthread_mutex_unlock(&lock);
            close(clientSocket);
            break;
        }
        // std::cout << server_data << std::endl;
        // requestHandlerThread = (pthread_t *)malloc(sizeof(pthread_t));
        // RequestType * rType = new RequestType(convertToString(server_data, status), clientSocket, status);
        // pthread_create(requestHandlerThread, NULL, handleRequest, rType);
        handleRequest(server_data, clientSocket, status);
        bzero(server_data, BUFFER_SIZE);
        // strcpy(response, "Login");
        // send(clientSocket, response, sizeof(response), 0);
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