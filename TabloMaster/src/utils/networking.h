#ifndef NETWORKING_H
#define NETWORKING_H

#include <string>
#include <thread>

// #include "udp_discovery.h"

class Networking
{
    public:
        Networking(std::string interface);
        void handleClientConnection(int serverSocket, int clientSocket);

    private:
        // UdpDiscovery udpDiscovery;
        // std::thread udpDiscoveryThread;
};

#endif
