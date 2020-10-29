#include<limits>
#include <ios> 
#include "./peer.h"


#define MAXFILEPATHSIZE 1024
#define MAXMESSAGESIZE 2048

using namespace std;
int main(int argc, char ** argv){


    pthread_t peerThread;
    std::string username, password;
    int option, tempSocket;
    int * peerSocketConnect;
    char message[MAXMESSAGESIZE] = "";
    char recvBuffer[MAXMESSAGESIZE] = "";
    char * baseFileName;
    char filePath[MAXFILEPATHSIZE] = "";
    pthread_t * receiveThread;

    if (argc < 3){
        cout << "Usage: ./main tracker_port_no peer_port_no" << endl;
        exit(1);
    }
    tracker_port = atoi(argv[1]);
    peer_port = atoi(argv[2]);


    int status = connectPeer(&tracker_socket, tracker_port);
    if (status == -1){
        cout << "Unable to connect to tracker" << endl;
        exit(1);
    }
    // Create a server socket that this peer listens to

    createServerSocket(&peer_socket, peer_port);

    pthread_create(&peerThread, NULL, listenFunc, NULL);
    
    // Send the port that this peer listens to
    strcpy(message, (SendPortCommand + ":").c_str());
    strcat(message, argv[2]);

    send(tracker_socket, message, strlen(message), 0);

    cout << "Enter an option" << endl;
    cin >> option;
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
    while(option != 0){
        bzero(message, MAXMESSAGESIZE);
        bzero(recvBuffer, MAXMESSAGESIZE);
        switch(option){
            case 1:
                strcpy(message, "Login");
                cout << "Enter username and password separated by space" << endl;
                cin >> username >> password;
                // send(tracker_socket, message, sizeof(message), 0);
                // recv(tracker_socket, &message, sizeof(message), 0);
                cout << username << ":" << password << endl;
                break;
            case 2:
                bzero(filePath, MAXFILEPATHSIZE);
                // std::cout << "Enter file path" << endl;
                // fgets(filePath, MAXFILEPATHSIZE, stdin);
                strcpy(filePath, "/home/arvindo/Downloads/Lab 5-PythonBasics.pdf\n");
                filePath[strlen(filePath) - 1] = 0;
                baseFileName = basename(filePath);
                strcpy(message, (UploadFileCommand + ":").c_str());
                strcat(message, baseFileName);
                send(tracker_socket, message, strlen(message), 0);
                FileMap[std::string(baseFileName)] = std::string(filePath);
                break;
            case 3:
                bzero(filePath, MAXFILEPATHSIZE);
                // std::cout << "Enter file name" << endl;
                // fgets(filePath, MAXFILEPATHSIZE, stdin);
                strcpy(filePath, "Lab 5-PythonBasics.pdf\n");
                filePath[strlen(filePath) - 1] = 0;
                strcpy(message, (DownloadFileCommand + ":").c_str());
                strcat(message, filePath);
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(message), 0);
                status = connectPeer(&tempSocket, stoi(recvBuffer));
                if (status == -1){
                    cout << "Could not connect to peer " << recvBuffer << endl;
                    continue;
                }
                receiveThread = (pthread_t *)malloc(sizeof(pthread_t));
                peerSocketConnect = (int *)(malloc(sizeof(int)));
                *peerSocketConnect = tempSocket;
                pthread_create(receiveThread, NULL, receiveDataFunc, peerSocketConnect);
                bzero(message, MAXMESSAGESIZE);
                strcpy(message, (RequestFilePeer + ":" + filePath).c_str());
                send(tempSocket, message, strlen(message), 0);
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