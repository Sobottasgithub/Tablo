#ifndef NETWORKING_H
#define NETWORKING_H

#include <string> 
#include <vector>
#include <map>

class Networking
{
    public:
        Networking();
        void sendMessage(int socket, const char* initialMessage);
        std::string recieveMessage(int socket);
        std::vector<std::string> getKeys(std::map<std::string, int> hashmap);
};

#endif
