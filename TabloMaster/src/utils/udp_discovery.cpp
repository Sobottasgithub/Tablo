#include "udp_discovery.h"

#include <server_session_controller.h>

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

void UdpDiscovery::udpDiscoveryCycle(std::string interface) {
    std::wcout << "Start Udp discovery..." << std::endl;

    ServerSessionController serverSessionController;
    std::string containerIP = serverSessionController.getLocalIpAddress(interface);
    std::string broadcastIP = serverSessionController.getBroadcastIpAddress();

    std::wcout << "Container IP: " << containerIP.c_str() << " | Broadcast IP: " << broadcastIP.c_str() << std::endl;

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
    struct sockaddr_in broadcast{}, receiverAddress{};
    const int port = 4000;

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
        std::wcout << "Setsockopt failed!" << std::endl;
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
        if (sendMessageTo(serverSocket, broadcast, containerIP.c_str()) != 0) {
            std::wcout << "Broadcast failed!" << std::endl;
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
            std::string data = receiveMessage(tcpNodeSocket);

            // Add ip to discovered Ip's if not already in vector
            if (serverSessionController.isValidIpV4(data) && std::find(nodeIPAddresses.begin(), nodeIPAddresses.end(), data) == nodeIPAddresses.end()) {
                nodeIPAddresses.push_back(data);
            }

            close(tcpNodeSocket);
        }
        usleep(100000);
    }
    close(serverSocket);
    std::wcout << "UDP socket closed..." << std::endl;
}

int UdpDiscovery::sendMessageTo(int socket, const sockaddr_in& broadcast, std::string payload) {
  if (sendto(socket, payload.data(), payload.size(), 0, (struct sockaddr*)&broadcast, sizeof(broadcast)) < 0) {
      std::wcout << "buffer: Sendto Failed!" << std::endl;
      return -1;
  }

  return 0;
}

std::string UdpDiscovery::receiveMessage(int socket) {
  std::string data;
  pollfd pfd{};
  pfd.fd = socket;
  pfd.events = POLLIN;

  int ret = poll(&pfd, 1, 10000);

  int bufferSize = 1024;
  if (ret > 0 && (pfd.revents & POLLIN)) {
      char* buffer = new char[bufferSize];
      ssize_t size = recv(socket, buffer, bufferSize, 0);
      
      data = buffer;
    
      if (size <= 0) return data;
      return data;
  }
  return data;
}

std::vector<std::string> UdpDiscovery::getNodeAdresses() {
    std::lock_guard<std::mutex> lock(mtx);
    return nodeIPAddresses;
}

void UdpDiscovery::removeNodeAddress(std::string nodeAddress) {
    std::lock_guard<std::mutex> lock(mtx);
    nodeIPAddresses.erase(find(nodeIPAddresses.begin(), nodeIPAddresses.end(), nodeAddress));
}

