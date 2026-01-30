#ifndef libtabnet
#define libtabnet

#include <string>

namespace tabnet {
  struct Packet {
    int method;
    std::string payload;  
  };
  
  std::string getLocalIpAddress(std::string interface);
  std::string getBroadcastIpAddress();
  int sendMessage(int socket, int method, std::string payload);
  Packet receiveMessage(int socket);
  bool isValidIpV4(std::string &ipString);
}

#endif
