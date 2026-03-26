#include "node_session_controller.h"
#include "udp_discovery.h"

#include "tabnet.h"
#include "networking.h"
#include "methods.h"
#include "client_session_controller.h"

#include <cstddef>
#include <iostream>
#include <cstring>
#include <memory>
#include <memory_resource>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <ctime>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <vector>
#include <algorithm>
#include <map>

Networking::Networking(std::string interface) {
    std::wcout << "Start socket..." << std::endl;
    udpDiscoveryThread = std::thread(&UdpDiscovery::udpDiscoveryCycle, &udpDiscovery, interface);

    // Client socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(4003);
    serverAddress.sin_addr.s_addr = inet_addr(tabnet::getLocalIpAddress(interface).c_str());

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, 5);

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        std::wcout << "New client connection!" << std::endl;
        clientConnections.push_back(std::thread(
           &Networking::handleClientConnection,
           this,
           serverSocket, clientSocket                             
        ));
    }
}

void Networking::handleClientConnection(int serverSocket, int clientSocket) {
    ClientSessionManager clientSessionManager(clientSocket);

    std::thread clientThread(
        &ClientSessionManager::sessionControllerCycle,
        &clientSessionManager
    );
    
    // TCP
    std::vector<std::string> nodeIps;
    int responseCode = 0;
    while (clientSessionManager.isConnected()) {
        std::vector<std::string> newNodeIps = udpDiscovery.getNodeAdresses();
        nodeIps = getIps(); 

        // sort vectors to compare them
        std::sort(newNodeIps.begin(), newNodeIps.end());
        std::sort(nodeIps.begin(), nodeIps.end());
        
        if(newNodeIps.size() == 0) {
            std::wcout << "No connected nodes!" << std::endl;
            if (nodeIps.size() > 0) {
                // Close connections with nodes that dont exist anymore
                for (int index = 0; index < nodeIps.size(); index++) {
                    std::wcout << "Close connection: " << nodeIps[index].c_str() << std::endl;
                    close(getConnectionAtIp(nodeIps[index])->socket);
                    removeConnectionAtIp(nodeIps[index]);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else if(newNodeIps == nodeIps) {
            bool nodeShutdown = false;
            while (!nodeShutdown) {
                // TODO: Replace with real distribution later
                std::vector<tabnet::Packet> orders;
                while(clientSessionManager.hasOrder()) {
                    orders.push_back(clientSessionManager.popOrder());
                }
                
                for (int i = 0; i < connections.size(); i++) {
                    Connection* currentConnection = &connections[i];

                    for (int j = 0; j < orders.size(); j++) {
                        currentConnection->controller->pushOrder(orders[j]);
                    }

                    while (currentConnection->controller->hasSolution()) {
                        clientSessionManager.pushSolution(
                            currentConnection->controller->popSolution()
                        );
                    }
                }
            }
        } else if (newNodeIps != nodeIps) {
            std::wcout << "New nodes!" << std::endl;

            // Create connections with nodes
            for (int index = 0; index < newNodeIps.size(); index++) {
                // If newNodeIps[index] not in nodeIps (keys of connections)
                if(std::find(nodeIps.begin(), nodeIps.end(), newNodeIps[index]) == nodeIps.end()) {
                    // Create new connection
                    int newSocket = socket(AF_INET, SOCK_STREAM, 0);

                    sockaddr_in nodeAddress;
                    nodeAddress.sin_family = AF_INET;
                    nodeAddress.sin_port = htons(4004);
                    nodeAddress.sin_addr.s_addr = inet_addr(newNodeIps[index].c_str());

                    if(connect(newSocket, (struct sockaddr*) &nodeAddress, sizeof(nodeAddress)) == -1) {
                        udpDiscovery.removeNodeAddress(newNodeIps[index]);
                        continue;
                    }

                    tabnet::Packet responseCode = tabnet::receiveMessage(newSocket);

                    // handshake compleate
                    if(responseCode.method == METHODS::success) {                        
                        auto nodeSessionController = std::make_unique<NodeSessionController>();

                         std::thread nodeSessionCycleThread(
                            &NodeSessionController::sessionControllerCycle,
                            nodeSessionController.get(),
                            newSocket
                        );

                        connections.emplace_back(Connection{
                            newNodeIps[index],
                            newSocket,
                            std::move(nodeSessionCycleThread),
                            std::move(nodeSessionController)
                        });
                                                
                        std::wcout << "Connection established: " << responseCode.method << " at ip: " << newNodeIps[index].c_str() << " | Connections.size(): " << connections.size() << std::endl;
                    }
                }
            }
            std::wcout << "[CLOSE] Connections with nodes that dont exist anymore" << std::endl;   
            // Close connections with nodes that dont exist anymore
            for (int index = 0; index < nodeIps.size(); index++) {
                std::wcout << "Iterating over nodeIPS" << std::endl;
                if(std::find(newNodeIps.begin(), newNodeIps.end(), nodeIps[index]) == newNodeIps.end()) {
                    // NOTE: This is NOT tested! (yet)
                    std::wcout << "Close connection: " << nodeIps[index].c_str() << std::endl;
                    close(getConnectionAtIp(nodeIps[index])->socket);
                    removeConnectionAtIp(nodeIps[index]);
                    nodeIps.erase(nodeIps.begin() + index);
                }
            }
        } else {
            std::wcout << "unknown operation!" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    // Close node con
    for(int index = 0; index < connections.size(); index++) {
        close(getConnectionAtIp(nodeIps[index])->socket);
    }
    std::wcout << "Terminate handleClientConnection for " << clientSocket << std::endl;
    udpDiscoveryThread.join();
}

std::vector<std::string> Networking::getIps() {
    std::vector<std::string> keys;
    for (const Connection& con : connections) {
        keys.push_back(con.ip);
    }
    return keys;
}

void Networking::removeConnectionAtIp(std::string ip) {
    for (int index = 0; index < connections.size(); index++) {
        if (connections[index].ip == ip) {
            connections.erase(connections.begin() + index);
            return;
        }
    }
}

Networking::Connection* Networking::getConnectionAtIp(std::string ip) {
    for (int index = 0; index < connections.size(); index++) {
        if (connections[index].ip == ip) {
            return &connections[index];
        }
    }
    return nullptr;
}
