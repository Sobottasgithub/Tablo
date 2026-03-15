#ifndef NETWORKING_H
#define NETWORKING_H

#include <string> 
#include <vector>
#include <map>
#include <thread>

#include "udp_discovery.h"
#include "node_session_controller.h"

class Networking
{
    public:
        Networking(std::string interface);
        void handleClientConnection(int serverSocket, int clientSocket);

    private:
        struct Connection {
            std::string ip;
            int socket;
            std::thread* controllerThread;
            NodeSessionController* controller;            
        };

        std::vector<Connection> connections;
        std::vector<std::thread> clientConnections;
        UdpDiscovery udpDiscovery;
        std::thread udpDiscoveryThread;

        std::vector<std::string> getIps();
        void removeConnectionAtIp(std::string ip);
        Connection getConnectionAtIp(std::string ip);
};

#endif
