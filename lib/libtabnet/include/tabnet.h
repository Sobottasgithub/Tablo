#ifndef libtabnet
#define libtabnet

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>

namespace tabnet {
  struct Packet {
    int method;
    std::string payload;  
  };
  
  std::string getLocalIpAddress(std::string interface);
  std::string getBroadcastIpAddress();
  int sendMessage(int socket, int method, std::string payload);
  int sendPacket(int socket, tabnet::Packet packet);
  int sendMessageTo(int socket, const sockaddr_in& broadcast, int method, std::string payload);
  Packet receiveMessage(int socket);
  bool isValidIpV4(std::string &ipString);
  bool isNumeric(const std::string& string);
}

#endif
