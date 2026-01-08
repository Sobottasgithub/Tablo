#ifndef NETWORKING_H
#define NETWORKING_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <bits/stdc++.h>
#include <vector>
#include <string>
#include "network_helpers.h"

class Networking
{
    public:
        Networking(std::string interface);
        void handleUdpDiscovery(std::string interface);
        void handleClientConnection(int serverSocket, int clientSocket);

    private:
        NetworkHelpers networkHelpers;
        int serverSocket;
        std::vector<std::thread> threadCollection;
        std::thread udpThread;
};

#endif
