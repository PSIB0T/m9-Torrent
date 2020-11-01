#include "./tracker_global.h"

FileInfo::FileInfo(){};

FileInfo::FileInfo(std::string fname, std::string fileSize){
    this->fileName = fname;
    this->fileSize = fileSize;
};

UserInfo::UserInfo(std::string username, std::string password){
    this->username = username;
    this->password = password;
}


int server_socket;
int port;
char address[20] = "0.0.0.0";
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
std::map <int, int> session;
std::map<std::string, UserInfo *> userDirectory;
std::map <std::string, struct FileInfo> FileMap;
