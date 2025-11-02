
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
    std::string containerIp = UdpDiscovery::getLocalIpAddress();
    std::wcout << "Container IP: " << containerIp.c_str() << endl;
    std::string broadcast_ip = UdpDiscovery::getBroadcastIpAddress();

    if (broadcast_ip.empty()) {
        std::cerr << "Failed to find broadcast IP!" << endl;
    }
    
    int sockfd;
    struct sockaddr_in dest{}, recvAddr{};
    const int port = 8888;
    const char* message = "100";
    const int bufferSize = 1024;
    char buffer[bufferSize];

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        wcout << "ERROR 2" << endl;
    }

    // Enable broadcast
    int broadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("setsockopt");
        close(sockfd);
        wcout << "ERROR 3" << endl;
    }

    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        close(sockfd);
        
    }

    // Bind socket to listen for responses
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    recvAddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&recvAddr, sizeof(recvAddr)) < 0) {

    vector<std::string> s = {};
        perror("bind failed");
        close(sockfd);
    }
        

    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    inet_pton(AF_INET, broadcast_ip.c_str(), &dest.sin_addr);

    vector<std::string> s = {};
        
    while(true) {    
        if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&dest, sizeof(dest)) < 0) {
            perror("sendto");
            close(sockfd);
            wcout << "ERROR 4" << endl;
        } 

        std::wcout << "MESSAGE SEND!" << endl;


        bool getResponse = false;
        
        while(!getResponse) {
            

        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        sockaddr_in senderAddr{};
        socklen_t senderLen = sizeof(senderAddr);

        ssize_t recvLen = recvfrom(sockfd, buffer, bufferSize - 1, 0,
                                   (struct sockaddr*)&senderAddr, &senderLen);

        if (recvLen < 0) {
            perror("recvfrom (timeout or error)");
            getResponse = true;
        }

        buffer[recvLen] = '\0';  // Null-terminate received message

        char senderIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &senderAddr.sin_addr, senderIP, INET_ADDRSTRLEN);

        char localIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &recvAddr.sin_addr, localIP, INET_ADDRSTRLEN);

    
        std::wcout << "recieved response" << senderIP << "-----" << localIP << " container " << containerIp.c_str() << endl;


        std::string senderStr(senderIP);
        if(senderStr != containerIp.c_str() && strcmp(senderIP, localIP) != 0) {
            std::wcout << "CRITERIA BOTH MET!" << endl;
    
            getResponse = true; 
        

        std::wcout << "Received response! " << senderIP << ": " << buffer << endl;

        // Optional: respond again to sender
        std::string reply = "Server received message: " + std::string(senderIP);
        if (sendto(sockfd, reply.c_str(), reply.length(), 0,
                   (struct sockaddr*)&senderAddr, senderLen) < 0) {
            perror("sendto (response) failed");
        } else {
            std::wcout << "Sent confirmation to " << senderIP << endl;
        }

            
        if (std::find(s.begin(), s.end(), std::string(senderIP)) == s.end()) {
            std::wcout << "--- UNIQUE FOUND! ---" << endl;

            s.push_back(std::string(senderIP));
        }

        

        std::wcout << "--------------------IP's-----------------------" << endl;
        for (const std::string& ip : s) {
            std::cout << ip << endl;
            std::wcout << ip.c_str() << endl;
        }
        
        std::wcout << "-------------------end-IP's-----------------------" << endl;

        }
        }
    }
       
    close(sockfd);
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
