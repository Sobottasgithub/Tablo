#include "client_session_controller.h"

#include "tabnet.h"
#include "methods.h"

#include <mutex>
#include <string>
#include <map>
#include <iostream>
#include <regex>
#include <cstring>
#include <sys/socket.h>

ClientSessionManager::ClientSessionManager() {}

ClientSessionManager::ClientSessionManager(int socket) {
  setSocket(socket);
}

void ClientSessionManager::setSocket(int socket) {
  this->socket = socket;
}

void ClientSessionManager::sessionControllerCycle() {
  int responseCode = 0;
  
  while (responseCode >= 0) {
    // Recieve orders
    tabnet::Packet orderCount = tabnet::receiveMessage(this->socket);
    if (orderCount.method == Methods::size) {
      responseCode = tabnet::sendMessage(this->socket, Methods::success, "");
      
      for (int index = 0; index < std::stoi(orderCount.payload); index++) {
        orderCollection.push_back(tabnet::receiveMessage(this->socket));
        responseCode = tabnet::sendMessage(this->socket, Methods::success, "");
      }
    } else {
      responseCode = tabnet::sendMessage(this->socket, Methods::failed, "");
      std::wcout << "Something went wrong during receiving size!" << std::endl;
      std::wcout << "Got: " << orderCount.method << " instead of " << Methods::size << " (size)" << std::endl;
    }

    // Send solutions
    int solutionCollectionSize = solutionCollection.size();
    responseCode = tabnet::sendMessage(this->socket, Methods::size, std::to_string(solutionCollectionSize));
    if (solutionCollectionSize > 0) {
      if (tabnet::receiveMessage(this->socket).method == Methods::success) {
        for(int index = 0; index < solutionCollectionSize; index++) {
          responseCode = tabnet::sendMessage(this->socket, solutionCollection[0].method, solutionCollection[0].payload);
          if (tabnet::receiveMessage(this->socket).method == Methods::success) {
            solutionCollection.erase(solutionCollection.begin());
          } else {
            std::wcout << "Send of solution failed!" << std::endl;
          }
        }
      } else {
        std::wcout << "Send of size failed!" << std::endl;
      }
    }
  }
  connected = false;
}

bool ClientSessionManager::hasOrder() {
  std::lock_guard<std::mutex> lock(mtx);
  return !orderCollection.empty();
}

bool ClientSessionManager::isConnected() {
  std::lock_guard<std::mutex> lock(mtx);
  return connected;
}

tabnet::Packet ClientSessionManager::popOrder() {
  std::lock_guard<std::mutex> lock(mtx);
  if (!orderCollection.empty()) {
    tabnet::Packet firstOrder = orderCollection[0];
    orderCollection.erase(orderCollection.begin());  
    return firstOrder;
  }
  tabnet::Packet emptyPacket;
  return emptyPacket;
}

void ClientSessionManager::pushSolution(tabnet::Packet solution) {
  std::lock_guard<std::mutex> lock(mtx);
  solutionCollection.push_back(solution);
}

int ClientSessionManager::getOrderCollectionSize() {
  std::lock_guard<std::mutex> lock(mtx);
  return orderCollection.size();
}

bool ClientSessionManager::isNumeric(const std::string& string) {
    static const std::regex numberRegex(
        R"(^[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?$)"
    );
    return std::regex_match(string, numberRegex);
}
