#include "../include/tabnet.h"

#include "tabcrypt.h"

#include <stdatomic.h>
#include <string>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
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

  int sendMessage(int socket, std::string initialMessage) {
    std::string encryptedMessage = tabcrypt::encrypt(secret, initialMessage);
    const char *message = encryptedMessage.c_str();
    ssize_t n = send(socket, message, strlen(message), MSG_NOSIGNAL);
    if (n < 0) {
      if (errno == EPIPE || errno == ECONNRESET) {
        return -1;
      }
    }
    return 0;
  }


  std::string receiveMessage(int socket) {
      pollfd pfd{};
      pfd.fd = socket;
      pfd.events = POLLIN;

      int ret = poll(&pfd, 1, 10000);
      if (ret > 0 && (pfd.revents & POLLIN)) {
          char buffer[1024]{};
          ssize_t size = recv(socket, buffer, sizeof(buffer)-1, 0);
          if (size <= 0) return "";
          return tabcrypt::decrypt(secret, std::string(buffer, size));
      }
      return "";
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
}
