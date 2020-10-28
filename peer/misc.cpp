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

void handleRequest(char * message, int clientSocket){
    std::string messageString(message);
    std::string command = messageString.substr(0, messageString.find(":"));
    std::string data = messageString.substr(messageString.find(":") + 1, messageString.size());
    char buffer[BUFFER_SIZE];
    if (command == FileStreamRecv){
        const char * fStreamRecvEnd = FileStreamRecvEnd.c_str();
        std::string fileName = data;
        std::cout << "Downloading file " << fileName << std::endl;
        int fp = open(fileName.c_str(), O_WRONLY | O_CREAT, 0775);
        int fileSize, i = 0;
        while((fileSize = recv(clientSocket, &buffer, BUFFER_SIZE, 0) )> 0 && strcmp(buffer, fStreamRecvEnd) != 0){
            i++;
            write(fp, &buffer, BUFFER_SIZE);
            bzero(buffer, BUFFER_SIZE);
        }
        std::cout << "Called " << i << "times" << std::endl;
        close(fp);
        std::cout << fStreamRecvEnd << std::endl;
        std::cout << "Successfully received file" << std::endl;
    } else if (command == RequestFilePeer){
        std::cout << "Client has requested a file" << std::endl;
        std::string fileName = data;
        int fp = open(FileMap[fileName].c_str(), O_RDONLY);
        if (fp == -1){
            std::cout << "Could not find file" << std::endl;
            return;
        }
        bzero(buffer, BUFFER_SIZE);
        strcpy(buffer, (FileStreamRecv + ":" + fileName).c_str());
        send(clientSocket, buffer, sizeof(buffer), 0);
        bzero(buffer, BUFFER_SIZE);
        int i = 0, size;
        while((size = read(fp, &buffer, BUFFER_SIZE)) != 0){
            i++;
            send(clientSocket, buffer, sizeof(buffer), 0);
            bzero(buffer, BUFFER_SIZE);
        }
        std::cout << "Called " << i << "times" << std::endl;
        strcpy(buffer, (FileStreamRecvEnd).c_str());
        send(clientSocket, buffer, sizeof(buffer), 0);
        std::cout << "Successfully sent file" << std::endl;
        close(fp);
    } else {
        std::cout << messageString << std::endl;
    }
}


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
    char response[BUFFER_SIZE] = "";
    int status;
    std::cout << "Listening to connections for " << clientSocket << std::endl;
    while(1){
        status = recv(clientSocket, &server_data, sizeof(server_data), 0);
        bzero(response, BUFFER_SIZE);
        if (status <= 0){
            printf("Peer %d disconnected!\n",clientSocket);
            pthread_mutex_lock(&lock);
            session.erase(session.find(clientSocket));
            pthread_mutex_unlock(&lock);
            close(clientSocket);
            break;
        }
        handleRequest(server_data, clientSocket);
        // strcpy(response, "Login");
        // send(clientSocket, response, sizeof(response), 0);
    }
    return NULL;
}


void * listenFunc(void * arg){
    int * peerSocket;
    pthread_t * receiveThread;
    std::cout << "Listening for connections" << std::endl;
    while(1){
        listen(peer_socket, 1);
        int client_socket = accept(peer_socket, NULL, NULL);
        if (client_socket == -1){
            printf("Error connecting to client!\n");
            continue;
        }
        pthread_mutex_lock(&lock);
        session[client_socket] = 1;
        pthread_mutex_unlock(&lock);

        receiveThread = (pthread_t *)malloc(sizeof(pthread_t));
        printf("Connected to client %d\n", client_socket);

        peerSocket = (int *)malloc(sizeof(int));
        *peerSocket = client_socket;

        // Listen to peer messages if connection is established.
        pthread_create(receiveThread, NULL, receiveDataFunc, peerSocket);
    }
    return NULL;
}