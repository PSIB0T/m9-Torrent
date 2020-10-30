#ifndef _PGLOBALS
#define _PGLOBALS

#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>
#include <vector>
#include <string>

#define BUFFER_SIZE 4096
#define CHUNK_SIZE 1024
#define COMMAND_SIZE 100
#define MAX_BACKLOG_SIZE 20000

extern int tracker_socket;
extern int peer_socket;
extern int tracker_port;
extern int peer_port;
extern char address[20];
extern pthread_mutex_t lock;
extern std::map <int, int> session;
extern std::map <std::string, std::string> FileMap;
extern std::string SendPortCommand;
extern std::string UploadFileCommand;
extern std::string DownloadFileCommand;
extern std::string RequestFilePeer;
extern std::string FileStreamRecv;
extern std::string FileStreamRecvEnd;

#endif
