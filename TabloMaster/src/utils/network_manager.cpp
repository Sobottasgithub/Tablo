#include "network_manager.h"
#include "tablog.h"

#include <arrow/array/builder_base.h>
#include <arrow/compute/api_scalar.h>
#include <arrow/table.h>
#include <networking.h>
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
#include <cerrno>
#include <poll.h>
#include <variant>
#include <unordered_map>
#include <iterator>

NetworkManager::NetworkManager(std::string interface) {
    logger->log(tablog::INFO, "Start socket...");
    auto serverDiscovery = std::make_shared<tud::ServerDiscovery>(interface, 4000, 4001, "Tablo");
    std::thread serverDiscoveryThread([serverDiscovery]() {
      serverDiscovery->discoveryCycle();
    });
    this->udpDiscovery = std::move(serverDiscovery);
    
    ttp2::ServerSessionController tempServerSessionController;
    std::string containerIP = tempServerSessionController.getLocalIpAddress(interface);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(4003);
    serverAddress.sin_addr.s_addr = inet_addr(containerIP.c_str());

    int serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
       logger->log(tablog::ERROR, "Bind failed");
       return;
    }

    // Create epoll
    int epollFd = epoll_create1(0);
    if (epollFd == -1) {
        logger->log(tablog::ERROR, "Failed to create epoll");
    }
    // Set epoll action for server
    struct epoll_event serverEvents;
    serverEvents.events = EPOLLIN;
    serverEvents.data.fd = serverSocket;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &serverEvents) == -1) {
        logger->log(tablog::ERROR, "Failed to set epoll_ctl");
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
                logger->log(tablog::INFO, "New Client Socket: " + std::to_string(clientSocket));

                clientConnections.push_back(std::thread([this, serverSocket, clientSocket]() {
                      this->handleClientConnection(serverSocket, clientSocket);
                }));
            }
        }
    }

    if (serverDiscoveryThread.joinable()) {
        serverDiscoveryThread.join();
    }
}

void NetworkManager::handleClientConnection(int serverSocket, int clientSocket) {
    logger->log(tablog::INFO, "Handle client conn");
    std::vector<Nodes> nodeConnections;
    
    auto serverSessionController = std::make_shared<ttp2::ServerSessionController>(serverSocket, clientSocket);

    std::thread networkingSession([serverSessionController]() {
        serverSessionController->networkingSession();
    });

    // file parameters for node distribution logic
    int filePartitionCount = 0;
    int lastDelimiter = 0;
    int columnCount = 0;

    // merge logic
    std::unordered_map<int, std::vector<ttp2::ServerSessionController::Viewport>> viewports = {};
    while(serverSessionController->isConnected()) {
        // Establish new node connections
        std::vector<std::string> discoveredNodes = udpDiscovery->getDiscoveredAddresses();
        for (int newNodeIndex = 0; newNodeIndex < discoveredNodes.size(); newNodeIndex++) {
            bool isNew = true;
            for (int index = 0; index < nodeConnections.size(); index++) {
                if (nodeConnections[index].ip == discoveredNodes[newNodeIndex]) {
                    isNew = false;
                    break;
                }
            }

            if (isNew) {
                std::string nodeIpv4 = discoveredNodes[newNodeIndex];
                logger->log(tablog::INFO, "Create new node connection at " + nodeIpv4);
                
                int nodeSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

                sockaddr_in nodeAddress;
                nodeAddress.sin_family = AF_INET;
                nodeAddress.sin_port = htons(4004);
                nodeAddress.sin_addr.s_addr = inet_addr(nodeIpv4.c_str());

                int connectionResult = connect(nodeSocket, (struct sockaddr*) &nodeAddress, sizeof(nodeAddress));

                // Wait for node to connect
                if (connectionResult < 0) {
                    if (errno == EINPROGRESS) {
                        struct pollfd pfd;
                        pfd.fd = nodeSocket;
                        pfd.events = POLLOUT;

                        // Wait max 10 Seconds for connection
                        int pollResult = poll(&pfd, 1, 10000);

                        if (pollResult > 0) {
                            int socketError = 0;
                            socklen_t len = sizeof(socketError);
                            getsockopt(nodeSocket, SOL_SOCKET, SO_ERROR, &socketError, &len);

                            if (socketError != 0) {
                                logger->log(tablog::ERROR, "Node connection " + nodeIpv4 + " failed!");
                                continue;
                            }
                        } else {
                          logger->log(tablog::ERROR, "Node connection " + nodeIpv4 + " failed!");
                          continue;
                        }
                    } else {
                      logger->log(tablog::ERROR, "Node connection " + nodeIpv4 + " failed!");
                      continue;
                    }
                }

                std::shared_ptr<ttp2::ClientSessionController> clientSessionController = std::make_shared<ttp2::ClientSessionController>(nodeSocket);

                std::thread networkThread([clientSessionController]() {
                    clientSessionController->networkingSession();
                });
                networkThread.detach();

                Nodes newNode = {nodeIpv4, clientSessionController};
                nodeConnections.push_back(newNode);
                logger->log(tablog::INFO, "Done!");
            }
        }

        // Remove disconnected nodes
        for (int index = 0; index < nodeConnections.size(); index++) {
            if(!nodeConnections[index].node->isConnected()) {
                logger->log(tablog::INFO, "Node with ip: " + nodeConnections[index].ip + " disconnected");
                udpDiscovery->removeDiscoveredAddress(nodeConnections[index].ip);
                nodeConnections.erase(nodeConnections.begin() + index);
            }
        }

        // Handle common business        
        // Send request
        if (serverSessionController->hasRequest()) {
            while (serverSessionController->hasRequest()) {
                ttp2::ServerSessionController::Packet packet = serverSessionController->popRequest();
                logger->log(tablog::DEBUG, "Received packet id: " + std::to_string(packet.id));

                if (std::holds_alternative<ttp2::ServerSessionController::Standard>(packet.payload)) {
                    for (int index = 0; index < nodeConnections.size(); index++) {
                        nodeConnections[index].node->pushRequest(packet);
                    }
                } else if (std::holds_alternative<ttp2::ServerSessionController::File>(packet.payload)) {
                    // INFO: Column based distribution
                    ttp2::ServerSessionController::File file = std::get<ttp2::ServerSessionController::File>(packet.payload);
                    filePartitionCount = file.payload->num_rows() / nodeConnections.size();
                    lastDelimiter = file.payload->num_rows() - 1;
                    columnCount = file.payload->num_columns() -1;
                    logger->log(tablog::DEBUG, "File dimensions: x0-" + std::to_string(lastDelimiter) + " y0-" + std::to_string(columnCount));

                    for (int nodeIndex = 0; nodeIndex < nodeConnections.size(); nodeIndex++) {                    
                        std::vector<std::shared_ptr<arrow::Field>> fields;
                        std::vector<std::shared_ptr<arrow::ChunkedArray>> columns;

                        int delimiter = 0;
                        if (nodeIndex == nodeConnections.size()-1) {
                            // If the last batch is reached the remainder should be added
                            delimiter = lastDelimiter;   
                        } else {
                            delimiter = filePartitionCount*(nodeIndex+1) - 1;
                        }
                        logger->log(tablog::DEBUG, "File " + nodeConnections[nodeIndex].ip + ": start row: " + std::to_string(filePartitionCount*nodeIndex) + " >> end row: " + std::to_string(delimiter));
                        std::shared_ptr<arrow::Table> slicedRowTable = file.payload->Slice(filePartitionCount*nodeIndex, delimiter);

                        ttp2::ServerSessionController::Packet nodePacket;
                        nodePacket.id = packet.id;
                        ttp2::ServerSessionController::File nodeFilePacket;
                        nodeFilePacket.filePath = file.filePath;
                        nodeFilePacket.start = filePartitionCount*nodeIndex;
                        nodeFilePacket.end = delimiter;
                        nodeFilePacket.payload = slicedRowTable;
                        nodePacket.payload = nodeFilePacket;
                    
                        nodeConnections[nodeIndex].node->pushRequest(nodePacket);
                    }
                } else if (std::holds_alternative<ttp2::ServerSessionController::Viewport>(packet.payload)) {
                    ttp2::ServerSessionController::Viewport viewport = std::get<ttp2::ServerSessionController::Viewport>(packet.payload);

                    for (int nodeIndex = 0; nodeIndex < nodeConnections.size(); nodeIndex++) {
                        int delimiter = 0;
                        if (nodeIndex == nodeConnections.size()-1) {
                            // If the last batch is reached the remainder should be added
                            delimiter = lastDelimiter;   
                        } else {
                            delimiter = filePartitionCount*(nodeIndex+1) - 1;
                        }

                        // Check if node is in range
                        int nodeStartIndex = filePartitionCount*nodeIndex;
                        if(nodeStartIndex > viewport.xEnd || delimiter < viewport.xStart) {
                            continue;
                        }

                        // Correct viewport
                        // x
                        if (viewport.xStart > nodeStartIndex)
                            nodeStartIndex = viewport.xStart;

                        if (viewport.xEnd < delimiter)
                            delimiter = viewport.xEnd;

                        // y
                        if (viewport.yStart < 0)
                            viewport.yStart = 0;

                        if (viewport.yEnd > columnCount)
                            viewport.yEnd = columnCount;
                        
                        ttp2::ServerSessionController::Packet nodePacket;
                        nodePacket.id = packet.id;
                        ttp2::ServerSessionController::Viewport nodeViewportPacket;
                        nodeViewportPacket.yStart = viewport.yStart;
                        nodeViewportPacket.yEnd = viewport.yEnd;
                        // Real slice e.x. node 2 gets 55 to 99
                        nodeViewportPacket.xStart = nodeStartIndex;
                        nodeViewportPacket.xEnd = delimiter;
                        nodePacket.payload = nodeViewportPacket;

                        logger->log(tablog::DEBUG, "Viewport " + nodeConnections[nodeIndex].ip + ": start: " + std::to_string(nodeStartIndex) + " end: " + std::to_string(delimiter));

                        nodeConnections[nodeIndex].node->pushRequest(nodePacket);
                    }
                } else {
                    logger->log(tablog::CRITICAL, "Unknown payload type!");
                }
            }
        }

        // Receive response
        // TODO: Rewrite merge logic later!
        if (nodeConnections.size() > 1) {
            for (int index = 0; index < nodeConnections.size(); index++) {
                if(nodeConnections[index].node->hasResponse()) {
                    ttp2::ServerSessionController::Packet packet = nodeConnections[index].node->popResponse();

                    if (std::holds_alternative<ttp2::ServerSessionController::Viewport>(packet.payload)) {
                        ttp2::ServerSessionController::Viewport viewport = std::get<ttp2::ServerSessionController::Viewport>(packet.payload);

                        arrow::Status status = viewport.payload->Validate();
                        if (!status.ok()) {
                            // TODO: request viewport packet here again
                            logger->log(tablog::CRITICAL, "Corrupt viewport payload!");
                            logger->log(tablog::CRITICAL, "xStart: " + std::to_string(viewport.xStart) + " xEnd: " + std::to_string(viewport.xEnd) +
                                                          "\nyStart: " + std::to_string(viewport.yStart) + " yEnd: " + std::to_string(viewport.yEnd));
                            
                            continue;
                        }
                        
                        if(viewports.find(packet.id) != viewports.end()) {
                            viewports[packet.id].push_back(viewport);
                        } else {
                            viewports[packet.id] = {viewport};
                        }
                    } else {
                        serverSessionController->pushResponse(packet);
                    }
                }
            }

            // merge viewports
            std::unordered_map<int, std::vector<ttp2::Networking::Viewport>>::iterator viewportIterator = viewports.begin();
            std::unordered_map<int, std::vector<ttp2::ServerSessionController::Viewport>> remainingViewports = {};
            while (viewportIterator != viewports.end()) {
                if (viewportIterator->second.size() >= nodeConnections.size()) {
                    std::vector<ttp2::Networking::Viewport> sortedViewports = insertionSortViewportsByX(viewportIterator->second);
                    
                    std::vector<std::shared_ptr<arrow::Table>> viewports = {};
                    for (int index = 0; index < sortedViewports.size(); index++) {
                        logger->log(tablog::DEBUG, "xStart: " +  std::to_string(sortedViewports[index].xStart) + " xEnd: " +  std::to_string(sortedViewports[index].xEnd));

                        viewports.push_back(sortedViewports[index].payload);
                    }
                
                    std::shared_ptr<arrow::Table> resultViewport = *arrow::ConcatenateTables(viewports);
                    logger->log(tablog::DEBUG, "Concatenated viewport: \n" + resultViewport->ToString());
            
                    ttp2::ServerSessionController::Packet resultPacket;
                    resultPacket.id = viewportIterator->first;
                    ttp2::ServerSessionController::Viewport resultViewportPacket;
                    resultViewportPacket.yStart = sortedViewports[0].yStart;
                    resultViewportPacket.yEnd = sortedViewports[0].yEnd;
                    resultViewportPacket.xStart = sortedViewports[0].xStart;
                    resultViewportPacket.xEnd = sortedViewports[-1].xEnd;
                    resultViewportPacket.payload = resultViewport;
                    resultPacket.payload = resultViewportPacket;
                    serverSessionController->pushResponse(resultPacket);

                    // TODO: REMOVE ENTRY
                } else {
                    remainingViewports[viewportIterator->first] = viewportIterator->second;
                }
                viewportIterator++;
            }
            viewports.clear();
            viewports = remainingViewports;
        } else {
            while (nodeConnections[0].node->hasResponse()) {
                serverSessionController->pushResponse(nodeConnections[0].node->popResponse());
            }
        }
    }

    // Disconnect node conns if client disconnects
    while (nodeConnections.size() > 0) {
        nodeConnections[0].node->disconnect();
        nodeConnections.erase(nodeConnections.begin());
    }
    
    networkingSession.join();
    logger->log(tablog::INFO, "Terminated");
}

std::vector<ttp2::Networking::Viewport> NetworkManager::insertionSortViewportsByX(std::vector<ttp2::Networking::Viewport> viewports) {
    for (int index = 1; index < viewports.size()-1; index++) {
        ttp2::Networking::Viewport viewport = viewports[index];
        int pointerIndex = index - 1;

        while (pointerIndex >= 0 && viewports[pointerIndex].xStart > viewport.xStart) {
            viewports[pointerIndex + 1] = viewports[pointerIndex];
            pointerIndex--;
        }
        viewports[pointerIndex + 1] = viewport;
    }
    return viewports;
}
