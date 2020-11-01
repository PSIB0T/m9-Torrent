#include "./tracker_global.h"

#ifndef _TGROUP
#define _TGROUP

bool createGroup(std::string groupName, int clientSocket);
bool checkGroupExistence(std::string groupName);
bool addGroupJoinRequests(std::string groupName, std::string username);
std::string listJoinGroupRequests(std::string groupName, int clientSocket);
int getGroupAdminSockId(std::string groupName);
bool acceptGroupJoinRequest(std::string groupName, std::string username, int clientSocket);
std::string listAllGroups();

#endif