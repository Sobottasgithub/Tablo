#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <thread>
#include <vector>
#include <string>

#include <tablog_registry.h>
#include <tablog.h>

class NetworkManager
{
    public:
        NetworkManager(std::string interface, int maxConnections);
        void handleUdpDiscovery(std::string interface);
        void handleClientConnection(int serverSocket, int clientSocket);

    private:
        std::shared_ptr<tablog::Tablog> logger = tablog::TablogRegistry::getInstance().get("Tablo-Node");

        int serverSocket;
        std::vector<std::thread> threadCollection;
        std::thread udpThread;
};

#endif
