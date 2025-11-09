#include "udp_discovery.h"

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

using namespace std;

void UdpDiscovery::udpDiscoveryCycle() {
    wcout << "Start Udp discovery..." << endl;
    std::string containerIP = UdpDiscovery::getLocalIpAddress();
    std::string broadcastIP = UdpDiscovery::getBroadcastIpAddress();

    std::wcout << "Container IP: " << containerIP.c_str() << " | Broadcast IP: " << broadcastIP.c_str() << endl;

    if (containerIP.empty()) {
        std::cerr << "Failed to find container IP!" << endl;
        return;
    }

    if (broadcastIP.empty()) {
        std::cerr << "Failed to find broadcast IP!" << endl;
        return;
    }
        
    int serverSocket;
    const char* message = "100";
    struct sockaddr_in broadcast{}, receiverAddress{};
    const int port = 8888;
    const int bufferSize = 1024;
    char buffer[bufferSize];

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create Socket!");
    }
    // Enable broadcast
    int broadcastBind = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_BROADCAST, &broadcastBind, sizeof(broadcastBind)) < 0) {
        perror("Failed to enable broadcast!");
        close(serverSocket);
    }
    // Allow Socket to bind on address with TIME_WAIT
    int reuse = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Setsockopt failed!");
        close(serverSocket);
    }

    // Bind socket to listen for responses
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    receiverAddress.sin_port = htons(port);
    if (bind(serverSocket, (struct sockaddr*)&receiverAddress, sizeof(receiverAddress)) < 0) {
        perror("Bind failed of receiverAddress");
        close(serverSocket);
    }
    
    // clear garbage
    memset(&broadcast, 0, sizeof(broadcast));
    // prepare socket
    broadcast.sin_family = AF_INET;
    broadcast.sin_port = htons(port);
    inet_pton(AF_INET, broadcastIP.c_str(), &broadcast.sin_addr);
        
    while(true) { 
        // Send message   
        if (sendto(serverSocket, message, strlen(message), 0, (struct sockaddr*)&broadcast, sizeof(broadcast)) < 0) {
            perror("Send message failed! Closing socket...");
            close(serverSocket);
        } 

        // While not timedout & no response from Node
        bool hasResponse = false;
        while(!hasResponse) {

            struct timeval timeout;
            timeout.tv_sec = 5;
            timeout.tv_usec = 0;

            setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

            sockaddr_in clientAddress{};
            socklen_t clientMessageLength = sizeof(clientAddress);

            ssize_t clientMessage = recvfrom(serverSocket, buffer, bufferSize - 1, 0,
                                    (struct sockaddr*)&clientAddress, &clientMessageLength);

            // Timeout: exit loop
            if (clientMessage < 0) {
                hasResponse = true;
            }

            buffer[clientMessage] = '\0';  // Null terminate received message

            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, INET_ADDRSTRLEN);

            char localIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &receiverAddress.sin_addr, localIP, INET_ADDRSTRLEN);

            // Compare recieved ip to own to prevent handshake with Master
            std::string clientIPString(clientIP);
            if(clientIPString != containerIP.c_str() && strcmp(clientIP, localIP) != 0) {
                hasResponse = true; 
 
                // Add ip to discovered Ip's if not already in vector
                if (std::find(nodeIPAddresses.begin(), nodeIPAddresses.end(), std::string(clientIP)) == nodeIPAddresses.end()) {
                    nodeIPAddresses.push_back(std::string(clientIP));
                }
            }
        }
    }
    close(serverSocket);
    wcout << "UDP socket closed..." << endl;
}

std::vector<std::string> UdpDiscovery::getNodeAdresses() {
    std::lock_guard<std::mutex> lock(mtx);
    return nodeIPAddresses;
}

std::string UdpDiscovery::getLocalIpAddress() {
    struct ifaddrs *ifaddr = nullptr;

    // Get linked list of network interfaces
    if (getifaddrs(&ifaddr) == -1) {
        return "";
    }

    std::string result;

    // Iterate through interfaces
    for (auto *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            auto *addr = reinterpret_cast<struct sockaddr_in *>(ifa->ifa_addr);
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));

            // Docker containers typically use eth0
            if (std::string(ifa->ifa_name) == "eth0") {
                result = ip;
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
    return result;
}

std::string UdpDiscovery::getBroadcastIpAddress() {
    struct ifaddrs* ifaddr = nullptr;
    std::string broadcastIP;

    // Get network interfaces
    if (getifaddrs(&ifaddr) == -1) {
        return "";
    }

    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        // Only consider IPv4 interfaces that are up and support broadcast
        if (ifa->ifa_addr->sa_family == AF_INET &&
            (ifa->ifa_flags & IFF_BROADCAST) &&
            (ifa->ifa_flags & IFF_UP) &&
            !(ifa->ifa_flags & IFF_LOOPBACK)) {

            // Ensure the broadcast address exists
            if (ifa->ifa_broadaddr) {
                struct sockaddr_in* bcast =
                    reinterpret_cast<struct sockaddr_in*>(ifa->ifa_broadaddr);
                char ip[INET_ADDRSTRLEN];
                if (inet_ntop(AF_INET, &(bcast->sin_addr), ip, INET_ADDRSTRLEN)) {
                    broadcastIP = ip;
                    break; // stop at the first valid one
                }
            }
        }
    }

    freeifaddrs(ifaddr);
    return broadcastIP;
}
