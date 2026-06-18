#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <thread>
#include <vector>
#include <string>

class NetworkManager
{
    public:
        NetworkManager(std::string interface);
        void handleUdpDiscovery(std::string interface);
        void handleClientConnection(int serverSocket, int clientSocket);

    private:
        int serverSocket;
        std::vector<std::thread> threadCollection;
        std::thread udpThread;
};

#endif
