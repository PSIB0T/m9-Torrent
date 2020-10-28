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

struct FileInfo {
    std::string fileName;
    std::vector<int> peers;
    FileInfo();
    FileInfo(std::string fname);
};

extern int server_socket;
extern int port;
extern char address[20];
extern pthread_mutex_t lock;
extern std::map <int, int> session;
extern std::string SendPortCommand;
extern std::string UploadFileCommand;
extern std::string DownloadFileCommand;
extern std::map <int, int> session;
extern std::map <std::string, struct FileInfo> FileMap;

#endif
