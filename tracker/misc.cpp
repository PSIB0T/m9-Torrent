#include "./misc.h"

std::string convertToString(char * data, int size){
    std::string res = "";
    for (int i = 0; i < size; i++){
        res += data[i];
    }
    return res;
}

std::vector<std::string> tokenize(std::string input, std::string delimiter, int noOfTokens){
    std::vector<std::string> res;
    std::size_t tempPos;
    while(noOfTokens && (tempPos = input.find(delimiter))!= std::string::npos){
        res.push_back(input.substr(0, tempPos));
        input = input.substr(tempPos + 1, input.size());
        noOfTokens--;
    }
    res.push_back(input);
    return res;
}


void handleRequest(std::string messageString, int clientSocket){
    std::string command = messageString.substr(0, messageString.find(":"));
    std::string data = messageString.substr(messageString.find(":") + 1, messageString.size());
    std::vector<std::string> tokens;
    std::string fileOwner;
    if (command == SendPortCommand){
        pthread_mutex_lock(&lock);
        session[clientSocket] = stoi(data);
        pthread_mutex_unlock(&lock);
        std::cout << "Information about client " << data << " stored!" << std::endl;
    } else if (command == UploadFileCommand){
        tokens = tokenize(data, ";", 1);
        pthread_mutex_lock(&lock);
        if (FileMap.find(tokens[0]) == FileMap.end()){
            FileMap[tokens[0]] = FileInfo(tokens[0], tokens[1]);
        }
        FileMap[tokens[0]].peers.push_back(session[clientSocket]);
        pthread_mutex_unlock(&lock);
        std::cout << "Successfully updated file data for " << data << std::endl;
    } else if (command == DownloadFileCommand){
        pthread_mutex_lock(&lock);
        if (FileMap.find(data) == FileMap.end()){
            fileOwner = FileNotFoundCode + ";File not found";
        }else {
            fileOwner = FileMap[data].fileSize + ";" + std::to_string(FileMap[data].peers[0]);
        }
        pthread_mutex_unlock(&lock);
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

void disconnectSession(int clientSocket){
    printf("Peer %d disconnected!\n",clientSocket);
    pthread_mutex_lock(&lock);
    session.erase(session.find(clientSocket));
    pthread_mutex_unlock(&lock);
    close(clientSocket);
}

void * receiveDataFunc(void * arg){
    int clientSocket = *((int *) arg);
    free(arg);
    char server_data[BUFFER_SIZE] = "";
    int status, remainingData;
    std::string messageBacklog = "";
    bool isDisconnectStatus = false;
    int sepPosition;
    while(1){
        status = recv(clientSocket, &server_data, sizeof(server_data), 0);
        if (status <= 0){
            disconnectSession(clientSocket);
            break;
        }
        messageBacklog += convertToString(server_data, status);
        sepPosition = messageBacklog.find(":");
        std::string dataLen = messageBacklog.substr(0, messageBacklog.find(":"));
        remainingData = (int)dataLen.size() + (int)stoi(dataLen) - status;
        while (remainingData > 0){
            bzero(server_data, BUFFER_SIZE);
            status = recv(clientSocket, &server_data, remainingData, 0);
            if (status <= 0){
                disconnectSession(clientSocket);
                isDisconnectStatus = true;
                break;
            }
            messageBacklog += convertToString(server_data, status);
            remainingData -= status;
        }
        if (isDisconnectStatus)
            break;
        // std::cout << "Status is " << status << std::endl;
        handleRequest(messageBacklog.substr(sepPosition + 1, stoi(dataLen)), clientSocket);
        bzero(server_data, BUFFER_SIZE);
        messageBacklog.erase(0, dataLen.size() + stoi(dataLen));
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