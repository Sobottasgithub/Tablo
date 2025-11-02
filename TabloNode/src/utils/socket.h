#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <bits/stdc++.h>
#include <vector>

class Socket
{
    public:
        Socket();
        void handleUdpDiscovery();
        void handleClientConnection(int serverSocket, int clientSocket);
        void sendMessage(int socket, const char* initialMessage);
        std::string recieveMessage(int socket);

    private:
        int serverSocket;
        std::vector<std::thread> threadCollection;
        std::thread udpThread; 
};

#endif
