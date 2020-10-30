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
    std::string totalData, chunkString, chunkNoString;
    std::string metaData = ":" + FileStreamRecv + ":" + fileSocket->fileName + ";";
    std::cout << "In upload thread"<<std::endl;
    int flag = 1;
    setsockopt(fileSocket->clientSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
    while((status=read(fp, &chunk, CHUNK_SIZE)) != 0){
        // std::cout << "Uploading chunk!" << std::endl;
        chunkString = ";" + convertToString(chunk, status);
        chunkNoString = std::to_string(chunkNo);
        totalData = std::to_string(metaData.size() + chunkNoString.size() + chunkString.size()) + metaData + chunkNoString + chunkString;
        stringToCharArr(buffer, totalData);
        send(fileSocket->clientSocket, buffer, totalData.size(), 0);
        bzero(buffer, BUFFER_SIZE);
        std::cout << chunkNo << " " << totalData.size()<< std::endl;
        chunkNo++;
        // sleep(0.5);
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
    close(fp);
    struct FileSocket * fileSocket = new FileSocket(clientSocket, fileName);
    pthread_t * uploadThread = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(uploadThread, NULL, uploadFileThreadFunc, fileSocket);
}

void downloadFile(std::string data, int clientSocket){
    std::vector<std::string> tokens = tokenize(data, ";", 2);
    std::cout << tokens[1] << " " << tokens[2].size() << std::endl;
    char buffer[BUFFER_SIZE] = "";
    FILE * fp = fopen(tokens[0].c_str(), "a");
    stringToCharArr(buffer, tokens[2]);
    if (tokens[2][tokens[2].size() - 1] == '1')
        fwrite(buffer, tokens[2].size() - 1, 1, fp);
    else
        fwrite(buffer, tokens[2].size(), 1, fp);
    fclose(fp);
    // sleep(2);
    // const char * fStreamRecvEnd = FileStreamRecvEnd.c_str();
    // std::cout << "Downloading file " << fileName << std::endl;
    // FILE * fp = fopen(tokens[0].c_str(), "a");
    // int fileSize, i = 0;
    // while((fileSize = recv(clientSocket, &buffer, BUFFER_SIZE, 0) )> 0 && strcmp(buffer, fStreamRecvEnd) != 0){
    //     fwrite(buffer, 1, fileSize, fp);
    //     bzero(buffer, BUFFER_SIZE);
    // }
    // std::cout << "Successfully received file" << std::endl;
    // fclose(fp);
    // std::cout << fStreamRecvEnd << std::endl;
    // fflush(stdout);
}