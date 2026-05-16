#include "udp_discovery.h"

#include <server_session_controller.h>

#include <iostream>
#include <string>
#include <stdexcept>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/poll.h>

UdpDiscovery::UdpDiscovery(std::string interface) {
    std::wcout << "Start udp discovery..." << std::endl;

    ServerSessionController serverSessionController;
    std::string containerIP = serverSessionController.getLocalIpAddress(interface);

    int udpSocket;
    const int port = 4000; 
    char buffer[1024];
    
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        std::wcout << "Create socket failed!" << std::endl;
        return;
    }

    int broadcast = 1;
    setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    sockaddr_in nodeAddress{};
    nodeAddress.sin_family = AF_INET;
    nodeAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    nodeAddress.sin_port = htons(port);

    if (bind(udpSocket, (struct sockaddr*)&nodeAddress, sizeof(nodeAddress)) < 0) {
        std::wcout << "UDP Socket bind failed!" << std::endl;
        return;
    }
    std::wcout << "Listening on UDP port " << port << std::endl;
    while (true) {
        // Get UDP Discovery packet
        std::string masterIP = receiveMessage(udpSocket);
        if (serverSessionController.isValidIpV4(masterIP)) {
                // DEBUG ONLY:
                //std::wcout << masterIP.c_str() << std::endl;

                // Send response over TCP
                int recieveSocket = 0;
                struct sockaddr_in masterAddress;
                masterAddress.sin_family = AF_INET;
                masterAddress.sin_port = htons(4001);

                if ((recieveSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                    std::wcout << "Socket creation error" << std::endl;
                    continue;
                }
                if (inet_pton(AF_INET, masterIP.c_str(), &masterAddress.sin_addr) <= 0) {
                    std::wcout << "Address not supported" << std::endl;
                    continue;
                }
                if (connect(recieveSocket, (struct sockaddr*)&masterAddress, sizeof(masterAddress)) < 0) {
                    std::wcout << "Connection failed" << std::endl;
                    continue;
                }
                //send(recieveSocket, containerIP.c_str(), containerIP.size(), 0);
                int result = sendMessage(recieveSocket, containerIP.c_str());
                close(recieveSocket);
        } else {
            std::wcout << "Invalid ip!" << std::endl;
        }
    }
    close(udpSocket);
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

      std::wcout << "BUffer: "<< buffer << std::endl;
      
      data = buffer;
    
      if (size <= 0) return data;
      return data;
  }
  return data;
}

int UdpDiscovery::sendMessage(int socket, std::string payload) {
  if (send(socket, payload.data(), payload.size(), 0) < 0) {
      std::wcout << "buffer: Send Failed!" << std::endl;
      return -1;
  }

  return 0;
}

