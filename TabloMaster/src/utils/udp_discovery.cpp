#include "udp_discovery.h"

#include "tabnet.h"

#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <ctime>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <cstdlib>
#include <ifaddrs.h>
#include <net/if.h>
#include <vector>
#include <algorithm>
#include <fcntl.h>

using namespace std;

void UdpDiscovery::udpDiscoveryCycle(std::string interface) {
    wcout << "Start Udp discovery..." << endl;

    std::string containerIP = tabnet::getLocalIpAddress(interface);
    std::string broadcastIP = tabnet::getBroadcastIpAddress();

    std::wcout << "Container IP: " << containerIP.c_str() << " | Broadcast IP: " << broadcastIP.c_str() << endl;

    if (containerIP.empty()) {
        std::wcout << "Failed to find container IP!" << std::endl;
        return;
    }

    if (broadcastIP.empty()) {
        std::wcout << "Failed to find broadcast IP!" << std::endl;
        return;
    }

    // Init: server socket   
    int serverSocket;
    const char* message = containerIP.c_str();
    struct sockaddr_in broadcast{}, receiverAddress{};
    const int port = 4000;
    const int bufferSize = 1024;
    char buffer[bufferSize];

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::wcout << "Failed to create Socket!" << std::endl;
        return;
    }
    // Enable broadcast
    int broadcastBind = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_BROADCAST, &broadcastBind, sizeof(broadcastBind)) < 0) {
        std::wcout << "Failed to enable broadcast!" << std::endl;
        close(serverSocket);
        return;
    }
    // Allow reuse
    int reuse = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::wcout << "Setsockopt failed!" << endl;
        close(serverSocket);
        return;
    }

    // clear garbage
    memset(&broadcast, 0, sizeof(broadcast));
    // prepare socket
    broadcast.sin_family = AF_INET;
    broadcast.sin_port = htons(port);

    if (inet_pton(AF_INET, broadcastIP.c_str(), &broadcast.sin_addr) <= 0) {
        std::wcout << "Invalid broadcast IP" << std::endl;
        close(serverSocket);
        return;
    }

    // Init recieve socket
    int tcpMasterSocket, tcpNodeSocket;
    struct sockaddr_in tcpAddress;
    int opt = 1;
    socklen_t addrlen = sizeof(tcpAddress);
    if ((tcpMasterSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::wcout << "socket failed" << std::endl;
        return;
    }
    if (setsockopt(tcpMasterSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::wcout << "Setsockopt failed!" << std::endl;
        return;
    }
    tcpAddress.sin_family = AF_INET;
    tcpAddress.sin_addr.s_addr = inet_addr(containerIP.c_str());
    tcpAddress.sin_port = htons(4001);
    if (bind(tcpMasterSocket, (struct sockaddr*)&tcpAddress, sizeof(tcpAddress)) < 0) {
        std::wcout << "Bind failed!" << std::endl;
        return;
    }
    if (listen(tcpMasterSocket, 3) < 0) {
        std::wcout << "Listen failed!" << std::endl;
        return;
    }

    while(true) { 
        // Send message   
        if (sendto(serverSocket, message, strlen(message), 0, (struct sockaddr*)&broadcast, sizeof(broadcast)) < 0) {
            std::wcout << "Broadcast failed!" << std::endl;
            close(serverSocket);
            return;
        }

        // Accept incoming connection
        int flags = fcntl(tcpMasterSocket, F_GETFL, 0);
        fcntl(tcpMasterSocket, F_SETFL, flags | O_NONBLOCK);

        int tcpNodeSocket = -1;
        time_t start = time(nullptr);
        while (time(nullptr) - start < 5) {
            tcpNodeSocket = accept(tcpMasterSocket, (struct sockaddr*)&tcpAddress, (socklen_t*)&addrlen);
            if (tcpNodeSocket >= 0) break;
            usleep(100000);
        }

        if (tcpNodeSocket < 0) {
            // DEBUG ONLY:
            //std::wcout << "Timeout: no connection" << std::endl;
            continue;
        } else {
            ssize_t valread = read(tcpNodeSocket, buffer, sizeof(buffer));
            // DEBUG ONLY:
            //std::wcout << "Received: " << buffer << std::endl;

            // Add ip to discovered Ip's if not already in vector
            if (std::find(nodeIPAddresses.begin(), nodeIPAddresses.end(), std::string(buffer)) == nodeIPAddresses.end()) {
                nodeIPAddresses.push_back(std::string(buffer));
            }

            close(tcpNodeSocket);
        }
        usleep(100000);
    }
    close(serverSocket);
    wcout << "UDP socket closed..." << endl;
}

std::vector<std::string> UdpDiscovery::getNodeAdresses() {
    std::lock_guard<std::mutex> lock(mtx);
    return nodeIPAddresses;
}

