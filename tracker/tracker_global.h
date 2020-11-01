#ifndef _TGLOBALS
#define _TGLOBALS

#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <map>
#include <vector>
#include "./../global_commands.h"

struct FileInfo {
    std::string fileName;
    std::string fileSize;
    std::vector<int> peers;
    FileInfo();
    FileInfo(std::string fname, std::string filesize);
};

typedef struct UserInfo {
    std::string username;
    std::string password;
    int currentSessionId;
    UserInfo(std::string username, std::string password);

}UserInfo;

extern int server_socket;
extern int port;
extern char address[20];
extern pthread_mutex_t lock;
extern std::map <int, int> session;
extern std::map <int, int> session;
extern std::map <std::string, struct FileInfo> FileMap;

#endif
