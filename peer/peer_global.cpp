#include "./peer_global.h"


int tracker_socket;
int peer_socket;
int tracker_port;
int peer_port;
char address[20] = "0.0.0.0";
pthread_mutex_t lock;
std::map <int, int> session;
std::map <std::string, std::string> FileMap;
std::string SendPortCommand("PeerPort");
std::string UploadFileCommand = "UploadFile";
std::string DownloadFileCommand = "DownloadFile";
std::string RequestFilePeer = "RequestFilePeer";
std::string FileStreamRecv = "FileStreamRecv";
std::string FileStreamRecvEnd = "FileStreamRecvEnd";