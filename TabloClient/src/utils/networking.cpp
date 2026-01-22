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
#include "methods.h"

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
    tabnet::sendMessage(clientSocket, Methods::size, std::to_string(orderCollectionSize));
    tabnet::Packet response = tabnet::receiveMessage(clientSocket);
    if (response.method == Methods::success) {
      for(int index = 0; index < orderCollectionSize; index++) {
        std::map<int, std::string> order = orderCollection[0];
        orderCollection.erase(orderCollection.begin());
        int method = order.begin()->first;

        // Send data
        tabnet::sendMessage(clientSocket, method, order[method]);
        tabnet::Packet response = tabnet::receiveMessage(clientSocket);
        if (response.method == Methods::failed) {
          std::wcout << "Something went wrong while sending the order! Master response: " << response.payload.c_str() << std::endl;
        } else {
          std::wcout << "Expected: " << Methods::success << " (success) or " << Methods::failed << " (failed), but got " << response.method << std::endl;
          std::wcout << "With following payload" << response.payload.c_str() << std::endl;
        }
      }
    } else if (response.method == Methods::failed) {
      std::wcout << "Something went wrong while sending the size! Master response: " << response.payload.c_str() << std::endl;
    } else {
      std::wcout << "Expected: " << Methods::success << " (success) or " << Methods::failed << " (failed), but got " << response.method << std::endl;
      std::wcout << "With following payload" << response.payload.c_str() << std::endl;
    }

    //receive
    tabnet::Packet receivedPacket = tabnet::receiveMessage(clientSocket);
    if (receivedPacket.method == Methods::size) {
      int count = std::stoi(receivedPacket.payload);
      if (count != 0) {
        tabnet::sendMessage(clientSocket, Methods::success, "");
        for(int i = 0; i < count; i++) {
          tabnet::Packet response = tabnet::receiveMessage(clientSocket);
          if (response.method == Methods::response) {
            solutionCollection.push_back(response.payload);
            tabnet::sendMessage(clientSocket, Methods::success, "");
          } else {
            std::wcout << "Expected: " << Methods::response << " (response) got: " << receivedPacket.method << std::endl;
            tabnet::sendMessage(clientSocket, Methods::failed, "Expected method response");
          }
        }
      }
    } else {
      std::wcout << "Expected: " << Methods::size << " (size) got: " << receivedPacket.method << std::endl;
      tabnet::sendMessage(clientSocket, Methods::failed, "Expected method size");
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
  std::map<int, std::string> order;
  order[method] = content;
  orderCollection.push_back(order);
}

bool Networking::isNumeric(const std::string& string) {
    static const std::regex number_regex(
        R"(^[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?$)"
    );
    return std::regex_match(string, number_regex);
}

