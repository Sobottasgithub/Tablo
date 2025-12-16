#ifndef NETWORKING_H
#define NETWORKING_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <bits/stdc++.h>
#include <vector>
#include <string>

class Networking
{
    public:
        Networking(std::string interface);
        void handleUdpDiscovery(std::string interface);
        void handleClientConnection(int serverSocket, int clientSocket);
        void sendMessage(int socket, const char* initialMessage);
        std::string recieveMessage(int socket);

    private:
        int serverSocket;
        std::vector<std::thread> threadCollection;
        std::thread udpThread; 
};

#endif
