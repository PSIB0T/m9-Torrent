#include "./tracker_global.h"

FileInfo::FileInfo(){};

FileInfo::FileInfo(std::string fname){
    this->fileName = fname;
};


int server_socket;
int port;
char address[20] = "0.0.0.0";
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
std::map <int, int> session;
std::string SendPortCommand =  "PeerPort";
std::string UploadFileCommand = "UploadFile";
std::string DownloadFileCommand = "DownloadFile";
std::map <std::string, struct FileInfo> FileMap;
