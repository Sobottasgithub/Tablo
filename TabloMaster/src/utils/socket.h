#ifndef SOCKET_H
#define SOCKET_H

#include <string> 
#include <vector>
#include <map>

class Socket
{
public:
    Socket();
    void sendMessage(int socket, const char* initialMessage);
    std::string recieveMessage(int socket);
    std::vector<std::string> getKeys(std::map<std::string, int> hashmap);
};

#endif
