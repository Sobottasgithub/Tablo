
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

UdpDiscovery::UdpDiscovery() {
    std::string containerIP = UdpDiscovery::getLocalIpAddress();
    std::wcout << "Container IP: " << containerIP.c_str() << endl;
    std::string broadcastIp = UdpDiscovery::getBroadcastIpAddress();

    if (broadcastIP.empty()) {
        std::cerr << "Failed to find broadcast IP!" << endl;
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

    vector<std::string> nodeIPAddresses = {};
        
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

                std::wcout << "Received response! " << clientIP << ": " << buffer << endl;
 
                // Add ip to discovered Ip's if not already in vector
                if (std::find(nodeIPAddresses.begin(), nodeIPAddresses.end(), std::string(clientIP)) == nodeIPAddresses.end()) {
                    nodeIPAddresses.push_back(std::string(clientIP));
                }

                // Only for debug:
                std::wcout << "--------------------IP's-----------------------" << endl;
                for (const std::string& ip : nodeIPAddresses) {
                    std::wcout << ip.c_str() << endl;
                }
                std::wcout << "-------------------end-IP's-----------------------" << endl;
            }
        }
    }
    close(serverSocket);
} 


std::string UdpDiscovery::getLocalIpAddress() {
    struct ifaddrs *ifaddr;

    // Iterate through interfaces
    for (auto *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        
        if (ifa->ifa_addr->sa_family == AF_INET) {
            auto *addr = (struct sockaddr_in *)ifa->ifa_addr;
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));

            // Docker containers typically have "eth0" as the main interface
            if (std::string(ifa->ifa_name) == "eth0") {
                std::wstring wip(ip, ip + strlen(ip));
                return ip;
            }
        }
    }

    //freeifaddrs(ifaddr);
    return "";
}

std::string UdpDiscovery::getBroadcastIpAddress() {
    struct ifaddrs* ifaddr;
    std::string broadcastIp;

    // Iterate through interfaces...
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        // Check if IPv4 addresses and interfaces (up) support broadcasting
        if (ifa->ifa_addr->sa_family == AF_INET &&
            (ifa->ifa_flags & IFF_BROADCAST) &&
            (ifa->ifa_flags & IFF_UP) &&
            !(ifa->ifa_flags & IFF_LOOPBACK)) {

            struct sockaddr_in* socketAddress = (struct sockaddr_in*)ifa->ifa_broadaddr;
            char ip[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &(socketAddress->sin_addr), ip, INET_ADDRSTRLEN)) {
                broadcastIp = ip;
                break;
            }
        }
    }

    //freeifaddrs(ifaddr);
    return broadcastIp;

}
