#include <iostream>
#include <cstring>

#include <arpa/inet.h>
#include <mutex>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/poll.h>
#include <regex>

#include "networking.h"

Networking::Networking() {}

void Networking::networkingCycle(std::string tabloMaster) {
  // creating socket
  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

  // specifying address
  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(4003);
  serverAddress.sin_addr.s_addr = inet_addr(tabloMaster.c_str());

  // sending connection request
  connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

  int count = 0;
  while(true) {

    // WIP: TODO: implement Tablonet send and receive
    int orderCollectionSize = orderCollection.size();
    std::string orderCollectionSizeString = std::to_string(orderCollectionSize);
    const char* orderCount = orderCollectionSizeString.c_str();
    send(clientSocket, orderCount, strlen(orderCount), 0);
    recieveMessage(clientSocket);
    for(int index = 0; index < orderCollectionSize; index++) {
      std::map<std::string, std::string> order= orderCollection[0];
      orderCollection.erase(orderCollection.begin());
      std::string methodString = order.begin()->first;
      // WIP: This will be deleted when tablonet is implemented:
      const char* method = methodString.c_str();
      std::string data = order[methodString];

      const char* content = data.c_str();
      send(clientSocket, method, strlen(method), 0);
      recieveMessage(clientSocket);
      send(clientSocket, content, strlen(content), 0);
      recieveMessage(clientSocket);
      count ++;
    }

    //recieve
    const char* success = "100";
    std::string recievedMessage = recieveMessage(clientSocket);
    if (Networking::isNumeric(recievedMessage)) {
      int count = std::stoi(recievedMessage);
      if (count != 0) {
        send(clientSocket, success, strlen(success), 0);
        for(int i = 0; i < count; i++) {
          solutionCollection.push_back(recieveMessage(clientSocket));
          send(clientSocket, success, strlen(success), 0);
        }
      }
    } else {
      std::wcout << "Count failed!" << std::endl;
    }
  }
}

bool Networking::hasSolution() {
  std::lock_guard<std::mutex> lock(mtx);
  return !solutionCollection.empty();
}

std::string Networking::popSolution() {
  std::lock_guard<std::mutex> lock(mtx);
  std::string solution = solutionCollection[0];
  solutionCollection.erase(solutionCollection.begin());
  return solution;
}

void Networking::pushOrder(int method, std::string content) {
  std::lock_guard<std::mutex> lock(mtx);
  std::map<std::string, std::string> order;
  order[std::to_string(method)] = content;
  orderCollection.push_back(order);
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

bool Networking::isNumeric(const std::string& string) {
    static const std::regex number_regex(
        R"(^[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?$)"
    );
    return std::regex_match(string, number_regex);
}

