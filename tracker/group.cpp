#include "./group.h"


bool checkGroupExistence(std::string groupName){
    bool doesGroupExist = false;
    pthread_mutex_lock(&groupLock);
    if (GroupDirectory.find(groupName) != GroupDirectory.end())
        doesGroupExist = true;
    pthread_mutex_unlock(&groupLock);

    return doesGroupExist;
}


int getGroupAdminSockId(std::string groupName){
    int sockId;
    std::string groupAdmin;
    pthread_mutex_lock(&groupLock);
        groupAdmin = GroupDirectory[groupName]->groupAdmin;
    pthread_mutex_unlock(&groupLock);

    pthread_mutex_lock(&userLock);
        sockId = UserDirectory[groupAdmin]->currentSessionId;
    pthread_mutex_unlock(&userLock);

    return sockId;
}

bool addGroupJoinRequests(std::string groupName, std::string username){
    bool successfullyAdded = true;
    pthread_mutex_lock(&groupLock);
        if (GroupDirectory[groupName]->users.find(username) != GroupDirectory[groupName]->users.end()){
            successfullyAdded = false;
        } else {
            GroupDirectory[groupName]->pendingRequests.insert(username);
        }
    pthread_mutex_unlock(&groupLock);
    return successfullyAdded;
}

bool createGroup(std::string groupName, int clientSocket){
    bool groupCreateSuccess = true;
    std::string username;
    pthread_mutex_lock(&groupLock);
    if (GroupDirectory.find(groupName) != GroupDirectory.end()){
        groupCreateSuccess = false;
    } else {
        pthread_mutex_lock(&lock);
        username = session[clientSocket];
        pthread_mutex_unlock(&lock);

        GroupDirectory[groupName] = new GroupInfo(groupName, username);
        GroupDirectory[groupName]->users.insert(username);
    }
    pthread_mutex_unlock(&groupLock);

    return groupCreateSuccess;
}

std::string listAllGroups(){
    std::string response = "";
    pthread_mutex_lock(&groupLock);
        for (auto a : GroupDirectory)
            response += a.first;
    pthread_mutex_unlock(&groupLock);

    return response;
}

bool acceptGroupJoinRequest(std::string groupName, std::string username, int clientSocket){
    bool isAdded = true;
    std::set<std::string>::iterator position;
    std::string requester, response;
        pthread_mutex_lock(&lock);
        requester = session[clientSocket];
    pthread_mutex_unlock(&lock);

    pthread_mutex_lock(&groupLock);
        if (GroupDirectory[groupName]->groupAdmin != requester || (position = GroupDirectory[groupName]->pendingRequests.find(username)) == GroupDirectory[groupName]->pendingRequests.end()){
            isAdded = false;
        }else {
            GroupDirectory[groupName]->users.insert(username);
            GroupDirectory[groupName]->pendingRequests.erase(position);
        }
    pthread_mutex_unlock(&groupLock);

    return isAdded;
}

std::string listJoinGroupRequests(std::string groupName, int clientSocket){
    std::string requester, response = "";
    pthread_mutex_lock(&lock);
    requester = session[clientSocket];
    pthread_mutex_unlock(&lock);

    
    pthread_mutex_lock(&groupLock);
        if (GroupDirectory[groupName]->groupAdmin != requester){
            response = InvalidAuthCode;
        }else {
            for (std::string a: GroupDirectory[groupName]->pendingRequests){
                response += (a + ";");
            }
            if (response.size() > 0){
                response.pop_back();
            }
        }
    pthread_mutex_unlock(&groupLock);

    return response;
}