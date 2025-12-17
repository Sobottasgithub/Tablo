#include <iostream>
#include <cstring>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "networking.h"

using namespace std;

Networking::Networking() {
  std::wcout << "Start tablo client socket..." << endl;
}

Networking::Networking(std::string tabloMaster) {
  std::wcout << "tablo Master: " << tabloMaster.c_str() << endl;

  // creating socket
  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

  // specifying address
  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(4003);
  serverAddress.sin_addr.s_addr = inet_addr(tabloMaster.c_str());

  // sending connection request
  connect(clientSocket, (struct sockaddr*)&serverAddress,
          sizeof(serverAddress));

  // sending data
  const char* message = "Hello, server!";
  send(clientSocket, message, strlen(message), 0);

  // closing socket
  close(clientSocket);
}
