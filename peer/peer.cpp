#include<limits>
#include <ios> 
#include "./peer.h"


#define MAXFILEPATHSIZE 1024

using namespace std;
int main(int argc, char ** argv){


    pthread_t peerThread;
    std::string username, password, baseFilenameString, totalMessage, fileSize, groupName;
    int option, tempSocket;
    int * peerSocketConnect;
    char message[BUFFER_SIZE] = "";
    char recvBuffer[BUFFER_SIZE] = "";
    char * baseFileName;
    char filePath[MAXFILEPATHSIZE] = "";
    pthread_t * receiveThread;
    vector<string> tokens;
    FILE * fp;
    int fp_oldOpen;

    sem_init(&FileDownloadSemaphore, 0, 1);

    if (argc < 3){
        cout << "Usage: ./main tracker_port_no peer_port_no" << endl;
        exit(1);
    }
    tracker_port = atoi(argv[1]);
    peer_port = atoi(argv[2]);


    // Initialize thread pool
    for (int i = 0; i < THREAD_POOL_SIZE; i++){
        pthread_create(&request_thread_pool[i], NULL, handleRequestThread, NULL);
    }

    int status = connectPeer(&tracker_socket, tracker_port);
    if (status == -1){
        cout << "Unable to connect to tracker" << endl;
        exit(1);
    }
    // Create a server socket that this peer listens to

    createServerSocket(&peer_socket, peer_port);

    pthread_create(&peerThread, NULL, listenFunc, NULL);
    


    cout << "Enter an option" << endl;
    cin >> option;
    cin.ignore(numeric_limits<streamsize>::max(),'\n');

    // Messages are sent in the following format to tracker/other peers
    // DataSize(including command):Command:Parameters/Data(separated by ;)
    while(option != 0){
        bzero(message, BUFFER_SIZE);
        bzero(recvBuffer, BUFFER_SIZE);
        switch(option){
            case 1:
                totalMessage = ":" + CreateUserCommand + ":";
                // strcpy(message, (CreateUserCommand + ":").c_str());
                cout << "Enter username and password separated by space" << endl;
                cin >> username >> password;
                totalMessage += username + ";" + password;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;

                break;

            case 2:
                totalMessage = ":" + LoginCommmand + ":";
                cout << "Enter username and password separated by space" << endl;
                cin >> username >> password;
                totalMessage += username + ";" + password;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                if (tokens[0] == InvalidAuthCode){
                    cout << "Invalid username or password" << endl;
                    break;
                } else {
                    postLogin(argv[2], tracker_socket);
                    pthread_mutex_lock(&lock);
                    isLoggedIn = true;
                    pthread_mutex_unlock(&lock);
                }
                break;
            case 3:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                totalMessage = ":" + CreateGroupCommand + ":";
                cout << "Enter group name to be created" << endl;
                cin >> groupName;
                totalMessage += groupName;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;
                break;
            case 4:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                totalMessage = ":" + JoinGroupCommand + ":";
                cout << "Enter group name to join" << endl;
                cin >> groupName;
                totalMessage += groupName;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;
                break;
            
            case 6:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                totalMessage = ":" +ListJoinGroupRequestsCommand + ":";
                cout << "Enter group name to list join requests" << endl;
                cin >> groupName;
                totalMessage += groupName;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;
                break;
            
            case 7:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                totalMessage = ":" +RespondToJoinGroupRequestCommand + ":";
                cout << "Enter group name, user name whose request you want to approve" << endl;
                cin >> groupName >> username;
                totalMessage += groupName + ";" + username;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;
                break;

            case 8:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                totalMessage = ":" + ListAllGroupsCommand + ":";
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;
                break;

            case 10:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                bzero(filePath, MAXFILEPATHSIZE);
                // std::cout << "Enter file path" << endl;
                // fgets(filePath, MAXFILEPATHSIZE, stdin);
                strcpy(filePath, "/home/arvindo/Downloads/Assignment_3.pdf\n");
                filePath[strlen(filePath) - 1] = 0;

                fp = fopen(filePath, "r");
                if (fp == NULL){
                    cout << "File unavailable" << endl;
                    break;
                }

                fseek(fp, 0, SEEK_END);
                fileSize = to_string(ftell(fp));
                fclose(fp);


                baseFileName = basename(filePath);
                baseFilenameString = std::string(baseFileName);

                totalMessage = baseFilenameString + ";" + fileSize;
                strcpy(message, (to_string(totalMessage.size() + UploadFileCommand.size() + 2) + ":" + UploadFileCommand + ":").c_str());
                strcat(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);

                pthread_mutex_lock(&lock);
                FileMap[baseFilenameString] = std::string(filePath);
                pthread_mutex_unlock(&lock);

                writerLock(&FileDownloadSemaphore);
                if (FileDownloadLockMap.find(baseFilenameString) == FileDownloadLockMap.end())
                    FileDownloadLockMap[baseFilenameString] = PTHREAD_MUTEX_INITIALIZER;
                writerUnlock(&FileDownloadSemaphore);

                break;
            case 11:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                bzero(filePath, MAXFILEPATHSIZE);
                // std::cout << "Enter file name" << endl;
                // fgets(filePath, MAXFILEPATHSIZE, stdin);
                strcpy(filePath, "Assignment_3.pdf\n");
                filePath[strlen(filePath) - 1] = 0;
                strcpy(message, (to_string(strlen(filePath) + DownloadFileCommand.size() + 2) + ":" + DownloadFileCommand + ":").c_str());
                strcat(message, filePath);
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(message), 0);
                
                tokens = tokenize(recvBuffer, ";", 1);

                if (tokens[0] == FileNotFoundCode){
                    cout << "File not found" << endl;
                    break;
                }

                pthread_mutex_lock(&lock);
                FileMap[std::string(filePath)] = std::string(filePath);
                pthread_mutex_unlock(&lock);

                writerLock(&FileDownloadSemaphore);
                if (FileDownloadLockMap.find(std::string(filePath)) == FileDownloadLockMap.end()){
                    FileDownloadLockMap[std::string(filePath)] = PTHREAD_MUTEX_INITIALIZER;
                    pthread_mutex_lock(&FileDownloadLockMap[std::string(filePath)]);
                    fp_oldOpen = open(filePath, O_WRONLY | O_CREAT, 0766);
                    fallocate(fp_oldOpen, 0, 0, stoi(tokens[0]));
                    close(fp_oldOpen);
                    pthread_mutex_unlock(&FileDownloadLockMap[std::string(filePath)]);
                }
                writerUnlock(&FileDownloadSemaphore);

                status = connectPeer(&tempSocket, stoi(tokens[1]));
                if (status == -1){
                    cout << "Could not connect to peer " << recvBuffer << endl;
                    continue;
                }
                receiveThread = (pthread_t *)malloc(sizeof(pthread_t));
                peerSocketConnect = (int *)(malloc(sizeof(int)));
                *peerSocketConnect = tempSocket;
                pthread_create(receiveThread, NULL, receiveDataFunc, peerSocketConnect);
                bzero(message, BUFFER_SIZE);
                strcpy(message, (to_string(strlen(filePath) + RequestFilePeer.size() + 2) + ":" + RequestFilePeer + ":" + filePath).c_str());
                send(tempSocket, message, strlen(message), 0);
                cout << "Exiting download command!" << endl;
                cin.ignore(numeric_limits<streamsize>::max(),'\n');

            default:
                cout << "Invalid command" << endl;
                break;
        }
        cout << "Enter an option" << endl;
        cin >> option;
        cin.ignore(numeric_limits<streamsize>::max(),'\n');
    }

    close(peer_socket);

    return 0;
}


bool checkLogin(){
    bool isLoggedInLocal;
    pthread_mutex_lock(&lock);
    isLoggedInLocal = isLoggedIn;
    pthread_mutex_unlock(&lock);
    return isLoggedInLocal;   
}

void postLogin(std::string port, int trackerSocket){
    // Send the port that this peer listens to
    std::string message = ":" + SendPortCommand + ":" + port;
    message = to_string(message.size()) + message;

    int status = send(trackerSocket, message.c_str(), message.size(), 0);
}