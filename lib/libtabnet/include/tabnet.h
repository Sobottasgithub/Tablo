#ifndef libtabnet
#define libtabnet

#include <string>

namespace tabnet {
  std::string getLocalIpAddress(std::string interface);
  std::string getBroadcastIpAddress();
  void sendMessage(int socket, const char* initialMessage);
  std::string receiveMessage(int socket);
  bool isValidIpV4(std::string &ipString);
}

#endif
