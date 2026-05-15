#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <string>
#include <thread>

// #include "udp_discovery.h"

class NetworkManager
{
    public:
        NetworkManager(std::string interface);
        void handleClientConnection(int serverSocket, int clientSocket);

    private:
        // UdpDiscovery udpDiscovery;
        // std::thread udpDiscoveryThread;
};

#endif
