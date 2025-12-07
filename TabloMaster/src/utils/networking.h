#ifndef NETWORKING_H
#define NETWORKING_H

#include <string> 
#include <vector>
#include <map>
#include <thread>

#include "udp_discovery.h"

class Networking
{
    public:
        Networking(std::string interface);
        void handleClientConnection(int serverSocket, int clientSocket);
        void sendMessage(int socket, const char* initialMessage);
        std::string recieveMessage(int socket);
        std::vector<std::string> getKeys(std::map<std::string, int> hashmap);

    private:
        std::vector<std::thread> clientConnections;
        UdpDiscovery udpDiscovery;
        std::thread udpDiscoveryThread;
};

#endif
