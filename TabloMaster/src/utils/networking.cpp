
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

#include "networking.h"
#include "methods.h"
#include "client_session_controller.h"

#include "udp_discovery.h"

using namespace std;

Networking::Networking(std::string interface) {
    std::wcout << "Start socket..." << endl;
    udpDiscoveryThread = std::thread(&UdpDiscovery::udpDiscoveryCycle, &udpDiscovery, interface);

    // Client socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(4003);
    serverAddress.sin_addr.s_addr = inet_addr(networkHelpers.getLocalIpAddress(interface).c_str());

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, 5);

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        std::wcout << "New client connection!" << endl;
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
    while (true) {
        std::vector<std::string> newNodeIps = udpDiscovery.getNodeAdresses();
        std::vector<std::string> nodeIps = Networking::getKeys(connections);

        // sort vectors to compare them
        std::sort(newNodeIps.begin(), newNodeIps.end());
        std::sort(nodeIps.begin(), nodeIps.end());
        
        if(newNodeIps.size() == 0) {
            wcout << "No connected nodes!" << endl;
            if (nodeIps.size() > 0) {
                // Close connections with nodes that dont exist anymore
                for (int index = 0; index < nodeIps.size(); index++) {
                    std::wcout << "Close connection: " << nodeIps[index].c_str() << endl;
                    close(connections[nodeIps[index]]);
                    connections.erase(nodeIps[index]);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else if(newNodeIps == nodeIps) {
            while (clientSessionManager.hasOrder()) {
                map<std::string, std::string> order = clientSessionManager.popOrder();
                wcout << "method: " << order.at("method").c_str() << " | content: "<< order.at("content").c_str() << endl;
                std::string method = order.at("method").c_str();
                std::string content = order.at("content").c_str();
                
                for(int index = 0; index < connections.size(); index++) {

                    int currentSocket = connections[nodeIps[index]];
                
                    networkHelpers.sendMessage(currentSocket, method.c_str());
                    std::string status = networkHelpers.receiveMessage(currentSocket);
                    if (!status.empty() && std::all_of(status.begin(), status.end(), ::isdigit) && std::stoi(status) == Methods::success) {
                        networkHelpers.sendMessage(currentSocket, content.c_str());
                        std::string status = networkHelpers.receiveMessage(currentSocket);
                        if (!status.empty() && std::all_of(status.begin(), status.end(), ::isdigit) && std::stoi(status) == Methods::success) {
                            std::string recievedData = networkHelpers.receiveMessage(currentSocket);
                            networkHelpers.sendMessage(currentSocket, std::to_string(Methods::success).c_str());
                            wcout << "Response: " << recievedData.c_str() << " | Node: " << nodeIps[index].c_str() << endl;
                            clientSessionManager.pushSolution(recievedData);
                        } else {
                            wcout << "Method " << method.c_str() << " failed with status: " << status.c_str() << endl;
                        }
                    } else {
                        wcout << "Method " << method.c_str() << " failed with status: " << status.c_str() << endl;
                    }
                }
            }
        } else if (newNodeIps != nodeIps) {
            wcout << "New nodes!" << endl;

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
                    connect(newSocket, (struct sockaddr*) &nodeAddress, sizeof(nodeAddress));

                    std::string respCode = networkHelpers.receiveMessage(newSocket);

                    // handshake compleate
                    if(!respCode.empty() && std::all_of(respCode.begin(), respCode.end(), ::isdigit) && std::stoi(respCode) == Methods::success) {
                        connections.insert({newNodeIps[index], newSocket});
                        std::wcout << "Connection established: " << respCode.c_str() << " at ip: " << newNodeIps[index].c_str() << endl;
                    }
                }
            }
                        
            // Close connections with nodes that dont exist anymore
            for (int index = 0; index < nodeIps.size(); index++) {
                if(std::find(newNodeIps.begin(), newNodeIps.end(), nodeIps[index]) == newNodeIps.end()) {
                    std::wcout << "Close connection: " << nodeIps[index].c_str() << endl;
                    close(connections[nodeIps[index]]);
                    connections.erase(nodeIps[index]);
                }
            }
        } else {
            wcout << "unknown operation!" << endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    
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
