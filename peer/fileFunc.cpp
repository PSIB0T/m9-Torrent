#include "./fileFunc.h"

FileSocket::FileSocket(int clientSocket, std::string fileName){
    this->clientSocket = clientSocket;
    this->fileName = fileName;
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

void stringToCharArr(char * res, std::string source){
    int size = source.size();
    for (int i = 0; i < size; i++){
        res[i] = source[i];
    }
}

std::string convertToString(char * data, int size){
    std::string res = "";
    for (int i = 0; i < size; i++){
        res += data[i];
    }
    return res;
}

void * uploadFileThreadFunc(void * arg){
    struct FileSocket * fileSocket = (struct FileSocket *) arg;
    int fp = open(FileMap[fileSocket->fileName].c_str(), O_RDONLY);
    char buffer[BUFFER_SIZE] = "";
    char chunk[CHUNK_SIZE] = "";
    // strcpy(buffer, (FileStreamRecv + ":" + fileSocket->fileName + ";").c_str());
    int messageLen = strlen(buffer);
    int chunkNo = 0;
    int status;
    std::string totalData;
    std::cout << "In upload thread"<<std::endl;
    int flag = 1;
    setsockopt(fileSocket->clientSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
    while((status=read(fp, &chunk, CHUNK_SIZE)) != 0){
        // std::cout << "Uploading chunk!" << std::endl;
        totalData = FileStreamRecv + ":" + fileSocket->fileName + ";" + std::to_string(chunkNo) + ";" + convertToString(chunk, status);
        stringToCharArr(buffer, totalData);
        send(fileSocket->clientSocket, buffer, totalData.size(), 0);
        // std::cout << "Status is " << status <<std::endl;
        bzero(buffer, BUFFER_SIZE);
        // std::cout << chunkNo << " " << totalData.size()<< std::endl;
        chunkNo++;
        sleep(0.5);
    }
    flag = 0;
    setsockopt(fileSocket->clientSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
    std::cout << "Uploaded the file!"<<std::endl;
    close(fp);
    free(arg);
    return NULL;
}


void uploadFile(std::string fileName, int clientSocket){
    std::cout << "Client has requested a file " << FileMap[fileName] << std::endl;
    int fp = open(FileMap[fileName].c_str(), O_RDONLY);
    if (fp == -1){
        std::cout << "Could not find file" << std::endl;
        return;
    }
    char buffer[BUFFER_SIZE] = "";
    bzero(buffer, BUFFER_SIZE);
    strcpy(buffer, (FileStreamRecv + ":" + fileName).c_str());
    send(clientSocket, buffer, sizeof(buffer), 0);
    bzero(buffer, BUFFER_SIZE);
    int i = 0, size;
    while((size = read(fp, &buffer, BUFFER_SIZE)) != 0){
        send(clientSocket, buffer, sizeof(buffer), 0);
        bzero(buffer, BUFFER_SIZE);
    }
    strcpy(buffer, (FileStreamRecvEnd).c_str());
    send(clientSocket, buffer, sizeof(buffer), 0);
    std::cout << "Successfully sent file" << std::endl;
}

void downloadFile(std::string fileName, int clientSocket){
    char buffer[BUFFER_SIZE] = "";
    const char * fStreamRecvEnd = FileStreamRecvEnd.c_str();
    std::cout << "Downloading file " << fileName << std::endl;
    FILE * fp = fopen(fileName.c_str(), "w");
    int fileSize, i = 0;
    while((fileSize = recv(clientSocket, &buffer, BUFFER_SIZE, 0) )> 0 && strcmp(buffer, fStreamRecvEnd) != 0){
        fwrite(buffer, 1, fileSize, fp);
        bzero(buffer, BUFFER_SIZE);
    }
    std::cout << "Successfully received file" << std::endl;
    fclose(fp);
    std::cout << fStreamRecvEnd << std::endl;
    fflush(stdout);
}