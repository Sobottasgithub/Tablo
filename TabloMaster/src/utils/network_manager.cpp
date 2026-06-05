#include "network_manager.h"

#include <server_session_controller.h>
#include <client_session_controller.h>
#include <server_discovery.h>

#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <memory>

NetworkManager::NetworkManager(std::string interface) {
    std::wcout << "Start socket..." << std::endl;
    auto serverDiscovery = std::make_shared<ServerDiscovery>(interface, 4000, 4001, "Tablo");
    std::thread serverDiscoveryThread([serverDiscovery]() {
      serverDiscovery->discoveryCycle();
    });
    this->udpDiscovery = std::move(serverDiscovery);
    
    ServerSessionController tempServerSessionController;
    std::string containerIP = tempServerSessionController.getLocalIpAddress(interface);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(4003);
    serverAddress.sin_addr.s_addr = inet_addr(containerIP.c_str());

    int serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
       std::wcout << "Bind failed!" << std::endl;
       return;
    }

    // Create epoll
    int epollFd = epoll_create1(0);
    if (epollFd == -1) {
        std::wcout << "Failed to create epoll!" << std::endl;
    }
    // Set epoll action for server
    struct epoll_event serverEvents;
    serverEvents.events = EPOLLIN | EPOLLOUT;
    serverEvents.data.fd = serverSocket;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &serverEvents) == -1) {
        std::wcout << "Failed to set epoll_ctl!" << std::endl;
        return;
    }

    listen(serverSocket, 5);
    std::vector<std::thread> clientConnections;
    while (true) {
        const int MAX_EVENTS = 10;
        struct epoll_event events[MAX_EVENTS];
        int epollRequestCount = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        
        for (int index = 0; index < epollRequestCount; ++index) {
            if (events[index].data.fd == serverSocket) {
                int clientSocket = accept4(serverSocket, nullptr, nullptr, SOCK_NONBLOCK);
                std::wcout << "New clientSocket: " << clientSocket << std::endl;

                clientConnections.push_back(std::thread([this, serverSocket, clientSocket]() {
                      this->handleClientConnection(serverSocket, clientSocket);
                }));
            }
        }
    }
}

void NetworkManager::handleClientConnection(int serverSocket, int clientSocket) {
    std::wcout << "Handle client conn" << std::endl;
    std::vector<Nodes> nodeConnections;
    
    auto serverSessionController = std::make_shared<ServerSessionController>(serverSocket, clientSocket);

    std::thread networkingSession([serverSessionController]() {
        serverSessionController->networkingSession();
    });

    while(serverSessionController->isConnected()) {
        // Establish new node connections
        std::vector<std::string> discoveredNodes = udpDiscovery->getDiscoveredAddresses();

        for (int newNodeIndex = 0; newNodeIndex < discoveredNodes.size(); newNodeIndex++) {
            bool isNew = true;
            for (int index = 0; index < nodeConnections.size(); index++) {
                if (nodeConnections[index].ip == discoveredNodes[index]) {
                    isNew = false;
                }
            }

            if (isNew) {
                std::wcout << "Create new conn!" << std::endl;
                std::string nodeIpv4 = discoveredNodes[newNodeIndex];
                
                int nodeSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

                sockaddr_in nodeAddress;
                nodeAddress.sin_family = AF_INET;
                nodeAddress.sin_port = htons(4004);
                nodeAddress.sin_addr.s_addr = inet_addr(nodeIpv4.c_str());

                int connectionResult = connect(nodeSocket, (struct sockaddr*) &nodeAddress, sizeof(nodeAddress));

                // Wait for server to accept
                if (connectionResult < 0 && errno != EINPROGRESS) {
                    std::wcout << "Connection failed!" << std::endl;
                    continue;
                }

                std::shared_ptr<ClientSessionController> clientSessionController = std::make_shared<ClientSessionController>(nodeSocket);

                std::thread networkThread([clientSessionController]() {
                    clientSessionController->networkingSession();
                });
                networkThread.detach();

                Nodes newNode = {nodeIpv4, clientSessionController};
                nodeConnections.push_back(newNode);
                std::wcout << "Done!" << std::endl;
            }
        }

        // Remove disconnected nodes
        for (int index = 0; index < nodeConnections.size(); index++) {
            if(!nodeConnections[index].node->isConnected()) {
                std::wcout << "node with ip: " << nodeConnections[index].ip.c_str() << " disconnected!" << std::endl;
                nodeConnections.erase(nodeConnections.begin() + index);
            }
        }

        // Handle common business
        // WARNING: This is only temporary. The distribution logic has to be rewritten later
        
        // Send request
        if (serverSessionController->hasRequest()) {
            ServerSessionController::Packet packet = serverSessionController->popRequest();
            std::wcout << "Received packet id: " << packet.id << std::endl;

            for (int index = 0; index < nodeConnections.size(); index++) {
                nodeConnections[index].node->pushRequest(packet);
            }
        }

        // Receive response
        for (int index = 0; index < nodeConnections.size(); index++) {
            while(nodeConnections[index].node->hasResponse()) {
                serverSessionController->pushResponse(nodeConnections[index].node->popResponse());
            }
        }
    }

    // Disconnect node conns if client disconnects
    for (int index = 0; index < nodeConnections.size(); index++) {
        nodeConnections[index].node->disconnect();
    }
    
    std::wcout << "Terminated!" << std::endl;
    networkingSession.detach();
    
    serverDiscoveryThread.join();
}
