#include "../include/tabnet.h"

#include "tabcrypt.h"
#include "methods.h"
#include "protobuf/transfer_protocol.pb.h"

#include <stdatomic.h>
#include <string>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <cstring>
#include <sys/poll.h>
#include <bits/stdc++.h>

std::string secret = "";

namespace tabnet {
  std::string getLocalIpAddress(std::string interface) {
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
              if (std::string(ifa->ifa_name) == interface) {
                  result = ip;
                  break;
              }
          }
      }

      freeifaddrs(ifaddr);
      return result;
  }

  std::string getBroadcastIpAddress() {
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

  int sendMessage(int socket, int method, std::string payload) {
    transferprotocol::SerializedPacket serializedData;
    serializedData.set_method(method);
    serializedData.set_payload(payload);

    size_t size = serializedData.ByteSizeLong(); 
    void *buffer = malloc(size);
    serializedData.SerializeToArray(buffer, size);
        
    //std::string encryptedMessage = tabcrypt::encrypt(secret, data);
    std::string stringSize = std::to_string(size);
    ssize_t s = send(socket, stringSize.c_str(), sizeof(stringSize.c_str()), MSG_NOSIGNAL);

    pollfd pfd{};
    pfd.fd = socket;
    pfd.events = POLLIN;

    int ret = poll(&pfd, 1, 10000);

    if (ret > 0 && (pfd.revents & POLLIN)) {
        char bufferSizeChar[32] = { 0 };
        ssize_t tempSize = recv(socket, bufferSizeChar, sizeof(bufferSizeChar), 0);
    }
    
    ssize_t n = send(socket, buffer, size, MSG_NOSIGNAL);
     if (n < 0) {
       if (errno == EPIPE || errno == ECONNRESET) {
         return -1;
       }
     }
    return 0;
  }


  Packet receiveMessage(int socket) {
      std::string dataString;
      transferprotocol::SerializedPacket SerializedData;
      Packet data;
      
      pollfd pfd{};
      pfd.fd = socket;
      pfd.events = POLLIN;

      int ret = poll(&pfd, 1, 10000);

      int bufferSize = 1024;
      if (ret > 0 && (pfd.revents & POLLIN)) {
          char bufferSizeChar[32] = { 0 };
          ssize_t tempSize = recv(socket, bufferSizeChar, sizeof(bufferSizeChar), 0);
          bufferSize = std::stoi(bufferSizeChar);
      }
      
      ssize_t s = send(socket, "100", sizeof("100"), MSG_NOSIGNAL);
      
      if (ret > 0 && (pfd.revents & POLLIN)) {
          char* buffer = new char[bufferSize];
          ssize_t size = recv(socket, buffer, bufferSize, 0);
          SerializedData.ParseFromArray(buffer, size);

          data = {SerializedData.method(), SerializedData.payload()};
          
          if (size <= 0) return data;
          return data;
      }
      return data;
  }

  bool isValidIpV4(std::string &ipString){
      if (ipString.size() < 7) return false;

      int count = 0;
      // Seperate Ip Octets
      std::stringstream stringStream(ipString);
      while (stringStream.good()){
          std::string octet;
          getline(stringStream, octet, '.');

          if (octet.size() > 1){
              if (octet[0] == '0')
                  return false;
          }

          for (int index = 0; index < octet.size(); index++){
              if (isalpha(octet[index]))
                  return false;
          }

          if (stoi(octet) > 255)
              return false;

          count++;
      }

      if (count != 4) return false;

      return true;
  }

  bool isNumeric(const std::string& string) {
    static const std::regex numberRegex(
        R"(^[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?$)"
    );
    return std::regex_match(string, numberRegex);
  }
}
