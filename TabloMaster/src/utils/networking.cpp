#include "udp_discovery.h"

#include "tabnet.h"
#include "networking.h"
#include "methods.h"
#include "client_session_controller.h"

#include <iostream>
#include <cstring>
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
    std::map<std::string, int> connections;
    std::vector<std::string> nodeIps;
    int responseCode = 0;
    while (clientSessionManager.isConnected()) {
        std::vector<std::string> newNodeIps = udpDiscovery.getNodeAdresses();
        nodeIps = Networking::getKeys(connections);

        // sort vectors to compare them
        std::sort(newNodeIps.begin(), newNodeIps.end());
        std::sort(nodeIps.begin(), nodeIps.end());
        
        if(newNodeIps.size() == 0) {
            std::wcout << "No connected nodes!" << std::endl;
            if (nodeIps.size() > 0) {
                // Close connections with nodes that dont exist anymore
                for (int index = 0; index < nodeIps.size(); index++) {
                    std::wcout << "Close connection: " << nodeIps[index].c_str() << std::endl;
                    close(connections[nodeIps[index]]);
                    connections.erase(nodeIps[index]);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else if(newNodeIps == nodeIps) {
            bool nodeShutdown = false;
            while (clientSessionManager.hasOrder() && !nodeShutdown) {
                tabnet::Packet order = clientSessionManager.popOrder();
                std::wcout << "method: " << order.method << " | content: " << order.payload.c_str() << std::endl;
                
                for(int index = 0; index < connections.size(); index++) {
                    int currentSocket = connections[nodeIps[index]];
                
                    responseCode = tabnet::sendMessage(currentSocket, order.method, order.payload);
                    tabnet::Packet status = tabnet::receiveMessage(currentSocket);
                    // TODO: needs a receive here maybe
                    if (status.method == Methods::success) {
                        tabnet::Packet receivedData = tabnet::receiveMessage(currentSocket);
                        responseCode = tabnet::sendMessage(currentSocket, Methods::success, "");
                        std::wcout << "Response: " << receivedData.method << " > " << receivedData.payload.c_str() << " | Node: " << nodeIps[index].c_str() << std::endl;
                        clientSessionManager.pushSolution(receivedData);
                    } else {
                        std::wcout << "Send failed!" << std::endl;
                    }

                    if (responseCode < 0) {
                        responseCode = 0;
                        udpDiscovery.removeNodeAddress(nodeIps[index]);
                        nodeShutdown = true;
                        break;
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
                    if(responseCode.method == Methods::success) {
                        connections.insert({newNodeIps[index], newSocket});
                        std::wcout << "Connection established: " << responseCode.method << " at ip: " << newNodeIps[index].c_str() << std::endl;
                    }
                }
            }
                        
            // Close connections with nodes that dont exist anymore
            for (int index = 0; index < nodeIps.size(); index++) {
                if(std::find(newNodeIps.begin(), newNodeIps.end(), nodeIps[index]) == newNodeIps.end()) {
                    std::wcout << "Close connection: " << nodeIps[index].c_str() << std::endl;
                    close(connections[nodeIps[index]]);
                    connections.erase(nodeIps[index]);
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
        close(connections[nodeIps[index]]);
    }
    std::wcout << "Terminate handleClientConnection for " << clientSocket << std::endl;
    udpDiscoveryThread.join();
}

std::vector<std::string> Networking::getKeys(std::map<std::string, int> hashmap) {
    std::vector<std::string> keys;
    // preallocate memory of exact size
    keys.reserve(hashmap.size());

    for (const auto& pair : hashmap) {
        keys.push_back(pair.first);
    }

    return keys;
}
