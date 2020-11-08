#include "./fileFunc.h"

int min(int a, int b){
    return a >= b?b:a;
}

FileSocket::FileSocket(int clientSocket, std::string fileName, int chunkNo){
    this->clientSocket = clientSocket;
    this->fileName = fileName;
    this->chunkNo = chunkNo;
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

// FileStreamRecv:filename;1;0;chunkString
// FileStreamRecv:filename;1;2048;chunkString

// chunk_size * chunk_no + 2048

// Send format: FileStreamRecv:filename;chunkNo;noOfBytesRecv;chunkString

void * uploadFileThreadFunc(void * arg){
    struct FileSocket * fileSocket = (struct FileSocket *) arg;

    readerLock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);
    const char * filePath = FileMap[fileSocket->fileName]->filePath.c_str();
    readerUnlock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);

    readerLock(&FileDownloadCount, &FileDownloadSemaphore, &FileDownloadMutex);
    readerLock(FileDownloadLockMap[fileSocket->fileName]);
    readerUnlock(&FileDownloadCount, &FileDownloadSemaphore, &FileDownloadMutex);

    int fp = open(filePath, O_RDONLY);

    lseek(fp, CHUNK_SIZE_GLOBAL * fileSocket->chunkNo, SEEK_SET);
    char buffer[BUFFER_SIZE] = "";
    char chunk[CHUNK_SIZE] = "";
    // strcpy(buffer, (FileStreamRecv + ":" + fileSocket->fileName + ";").c_str());
    int messageLen = strlen(buffer);
    int chunkNo = 0;
    int status, totalFileSize;
    std::string totalData, chunkString, noOfBytesReadString, chunkNoString;

    chunkNoString = std::to_string(fileSocket->chunkNo) + ";";

    std::string metaData = ":" + FileStreamRecv + ":" + fileSocket->fileName + ";" + chunkNoString;
    std::cout << "In upload thread"<<std::endl;

    int noOfBytesRead = 0;
    int remainingBytes = CHUNK_SIZE_GLOBAL;

    
    while(remainingBytes > 0 && (status=read(fp, &chunk, min(CHUNK_SIZE, remainingBytes))) != 0){
        // std::cout << "Uploading chunk!" << std::endl;
        chunkString = convertToString(chunk, status);
        noOfBytesReadString = std::to_string(noOfBytesRead) + ";";
        totalData = metaData + noOfBytesReadString + chunkString;
        totalData = std::to_string(totalData.size()) + totalData;
        stringToCharArr(buffer, totalData);
        send(fileSocket->clientSocket, buffer, totalData.size(), 0);
        bzero(buffer, BUFFER_SIZE);
        bzero(chunk, CHUNK_SIZE);
        std::cout << chunkNo << " " << totalData.size()<< std::endl;
        chunkNo++;
        noOfBytesRead += status;
        remainingBytes -= status;
        // sleep(0.5);
    }
    std::cout << "Uploaded the file!"<<std::endl;
    close(fp);

    readerLock(&FileDownloadCount, &FileDownloadSemaphore, &FileDownloadMutex);
    readerUnlock(FileDownloadLockMap[fileSocket->fileName]);
    readerUnlock(&FileDownloadCount, &FileDownloadSemaphore, &FileDownloadMutex);

    free(arg);
    return NULL;
}


void uploadFile(std::string fileName, int chunkNo,int clientSocket){
    std::cout << "Client has requested a file " << FileMap[fileName] << std::endl;
    readerLock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);
    const char * filePath = FileMap[fileName]->filePath.c_str();
    readerUnlock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);


    int fp = open(filePath, O_RDONLY);
    if (fp == -1){
        std::cout << "Could not find file" << std::endl;
        return;
    }
    close(fp);
    struct FileSocket * fileSocket = new FileSocket(clientSocket, fileName, chunkNo);
    pthread_t * uploadThread = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(uploadThread, NULL, uploadFileThreadFunc, fileSocket);
}

void downloadFile(std::string data, int clientSocket){
    std::vector<std::string> tokens = tokenize(data, ";", 3);
    std::string response;
    int filePos = stoi(tokens[1]) * CHUNK_SIZE_GLOBAL + stoi(tokens[2]);
    char buffer[BUFFER_SIZE] = "";
    int syncSocket;

    readerLock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);
    const char * filePath = FileMap[tokens[0]]->filePath.c_str();
    readerUnlock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);
    
    // Request lock for that particular file
    readerLock(&FileDownloadCount, &FileDownloadSemaphore, &FileDownloadMutex);
    writerLock(FileDownloadLockMap[tokens[0]]);
    readerUnlock(&FileDownloadCount, &FileDownloadSemaphore, &FileDownloadMutex);
    // std::cout << tokens[1] << " " << tokens[2] << std::endl;

    FILE * fp = fopen(filePath, "r+");

    fseek(fp, filePos, SEEK_SET);

    // std::cout << filePos << std::endl;
    int sizeToWrite = tokens[3][tokens[3].size() - 1] == '1'?tokens[3].size() - 1:tokens[3].size();


    stringToCharArr(buffer, tokens[3]);
    fwrite(buffer, sizeToWrite, 1, fp);
    fclose(fp);

    readerLock(&FileDownloadCount, &FileDownloadSemaphore, &FileDownloadMutex);
    writerUnlock(FileDownloadLockMap[tokens[0]]);
    readerUnlock(&FileDownloadCount, &FileDownloadSemaphore, &FileDownloadMutex);

    writerLock(&FileMapSemaphore);
        BitVectorType * bVectorType = FileMap[tokens[0]]->bitVector[loggedInUser]->at(stoi(tokens[1]));
        bVectorType->bytesReceived += sizeToWrite;
        if (bVectorType->bytesReceived >= bVectorType->chunkSize){
            bVectorType->bit = 1;
            for (auto a: FileMap[tokens[0]]->bitVector){
                if (a.first != loggedInUser){
                    response = ":" + SetIndividualBitVector + ":" + loggedInUser + ";" + tokens[0] + ";" + "1" + ";" + tokens[1];
                    response = std::to_string(response.size()) + response;
                    pthread_mutex_lock(&userLock);
                    syncSocket = UserDirectory[a.first]->currentSessionId;
                    pthread_mutex_unlock(&userLock);
                    send(syncSocket, response.c_str(), response.size(), 0);
                }
            }
        }
    writerUnlock(&FileMapSemaphore);
}