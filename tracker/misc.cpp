#include "./misc.h"


void handleRequest(char * message, int clientSocket){
    std::string messageString(message);
    std::string command = messageString.substr(0, messageString.find(":"));
    std::string data = messageString.substr(messageString.find(":") + 1, messageString.size());
    if (command == SendPortCommand){
        pthread_mutex_lock(&lock);
        session[clientSocket] = stoi(data);
        pthread_mutex_unlock(&lock);
        std::cout << "Information about client " << data << " stored!" << std::endl;
    } else if (command == UploadFileCommand){
        pthread_mutex_lock(&lock);
        if (FileMap.find(data) == FileMap.end()){
            FileMap[data] = FileInfo(data);
        } 
        FileMap[data].peers.push_back(session[clientSocket]);
        pthread_mutex_unlock(&lock);
        std::cout << "Successfully updated file data for " << data << std::endl;
    } else if (command == DownloadFileCommand){
        std::string fileOwner = std::to_string(FileMap[data].peers[0]);
        send(clientSocket, fileOwner.c_str(), fileOwner.size(), 0);
    }else {
        std::cout << "Client data is " << messageString << std::endl;
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
    int status;
    while(1){
        status = recv(clientSocket, &server_data, sizeof(server_data), 0);
        if (status <= 0){
            printf("Peer %d disconnected!\n",clientSocket);
            pthread_mutex_lock(&lock);
            session.erase(session.find(clientSocket));
            pthread_mutex_unlock(&lock);
            close(clientSocket);
            break;
        }
        std::cout << "Status is " << status << std::endl;
        handleRequest(server_data, clientSocket);
        bzero(server_data, BUFFER_SIZE);
    }
    return NULL;
}

void * listenFunc(void * arg){
    int * peerSocket;
    pthread_t * receiveThread;
    std::cout << "Listening for connections" << std::endl;
    while(1){
        listen(server_socket, 1);
        int client_socket = accept(server_socket, NULL, NULL);
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