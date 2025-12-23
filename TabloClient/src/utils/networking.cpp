#include <iostream>
#include <cstring>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/poll.h>

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
  connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

  // WIP: Will be compleatly rewritten: only for testing
  int count = 0;
  while(true) {
    const char* orderCount = "2";
    send(clientSocket, orderCount, strlen(orderCount), 0);
    recieveMessage(clientSocket);
    for(int index = 0; index < 2; index++) {
      const char* method = "401";
      std::string data = "test" + to_string(count);
      const char* content = data.c_str();
      send(clientSocket, method, strlen(method), 0);
      recieveMessage(clientSocket);
      send(clientSocket, content, strlen(content), 0);
      recieveMessage(clientSocket);
      count ++;
    }

    //recieve
    const char* success = "100";
    int count = std::stoi(recieveMessage(clientSocket));
    send(clientSocket, success, strlen(success), 0);
    for(int i = 0; i < count; i++) {
      std::wcout << recieveMessage(clientSocket).c_str() << endl;
      send(clientSocket, success, strlen(success), 0);
    }
  }
}

 std::string Networking::recieveMessage(int socket) {
    pollfd pfd{};
    pfd.fd = socket;
    pfd.events = POLLIN;

    int ret = poll(&pfd, 1, 10000);
    if (ret > 0 && (pfd.revents & POLLIN)) {
        char buffer[1024]{};
        ssize_t n = recv(socket, buffer, sizeof(buffer)-1, 0);
        if (n <= 0) return "";
        return std::string(buffer, n);
    }
    return "";
}
