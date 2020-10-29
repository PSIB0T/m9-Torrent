#include "./peer_global.h"


#ifndef _PFILEFUNC
#define _PFILEFUNC


struct FileSocket {
    int clientSocket;
    std::string fileName;
    FileSocket();
    FileSocket(int clientSocket, std::string fileName);
};

void uploadFile(std::string fileName, int clientSocket);
void downloadFile(std::string fileName, int clientSocket);
void substring(char * destination, char * source, int start, int len);
std::string convertToString(char * data, int size);
int find(char * str, char needle);

#endif