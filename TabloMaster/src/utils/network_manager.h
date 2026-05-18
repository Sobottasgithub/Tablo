#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <client_session_controller.h>
#include <memory>
#include <string>
#include <thread>

#include "udp_discovery.h"

class NetworkManager
{
    public:
        NetworkManager(std::string interface);
        void handleClientConnection(int serverSocket, int clientSocket);

    private:
        struct Nodes {
          std::string ip;
          std::shared_ptr<ClientSessionController> node;  
        };
        
        UdpDiscovery udpDiscovery;
        std::thread udpDiscoveryThread;
};

#endif
