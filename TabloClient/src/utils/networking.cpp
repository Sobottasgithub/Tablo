#include "networking.h"

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

#include "tabnet.h"

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

  while(true) {
    int orderCollectionSize = orderCollection.size();
    tabnet::sendMessage(clientSocket, std::to_string(orderCollectionSize));
    tabnet::receiveMessage(clientSocket);
    for(int index = 0; index < orderCollectionSize; index++) {
      std::map<std::string, std::string> order= orderCollection[0];
      orderCollection.erase(orderCollection.begin());
      std::string method = order.begin()->first;

      // Send method
      tabnet::sendMessage(clientSocket, method);
      tabnet::receiveMessage(clientSocket);
      // Send content
      tabnet::sendMessage(clientSocket, order[method]);
      tabnet::receiveMessage(clientSocket);
    }

    //receive
    const char* success = "100";
    std::string receivedMessage = tabnet::receiveMessage(clientSocket);
    if (Networking::isNumeric(receivedMessage)) {
      int count = std::stoi(receivedMessage);
      if (count != 0) {
        tabnet::sendMessage(clientSocket, success);
        for(int i = 0; i < count; i++) {
          solutionCollection.push_back(tabnet::receiveMessage(clientSocket));
          tabnet::sendMessage(clientSocket, success);
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

bool Networking::isNumeric(const std::string& string) {
    static const std::regex number_regex(
        R"(^[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?$)"
    );
    return std::regex_match(string, number_regex);
}

