#include "client_session_controller.h"

#include "tabnet.h"

#include <mutex>
#include <string>
#include <map>
#include <iostream>
#include <regex>
#include <cstring>
#include <sys/socket.h>

#include "methods.h"

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
    std::string orderCountString = tabnet::receiveMessage(this->socket);
    responseCode = tabnet::sendMessage(this->socket, std::to_string(Methods::success).c_str());
    if (isNumeric(orderCountString)) {
      int orderCount = std::stoi(orderCountString);
      for (int index = 0; index < orderCount; index++) {
        std::string method  = tabnet::receiveMessage(this->socket);
        responseCode = tabnet::sendMessage(this->socket, std::to_string(Methods::success).c_str());
        std::string content = tabnet::receiveMessage(this->socket);
        responseCode = tabnet::sendMessage(this->socket, std::to_string(Methods::success).c_str());
        orderCollection.push_back({ {"method", method}, {"content", content} });
      }
    }

    // Send solutions
    int solutionCollectionSize = solutionCollection.size();
    responseCode = tabnet::sendMessage(this->socket, std::to_string(solutionCollectionSize).c_str());
    if (solutionCollectionSize > 0) {
      tabnet::receiveMessage(this->socket);
      for(int index = 0; index < solutionCollectionSize; index++) {
        responseCode = tabnet::sendMessage(this->socket, solutionCollection[0].c_str());
        tabnet::receiveMessage(this->socket);
        solutionCollection.erase(solutionCollection.begin());
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

std::map<std::string, std::string> ClientSessionManager::popOrder() {
  std::lock_guard<std::mutex> lock(mtx);
  if (!orderCollection.empty()) {
    std::map<std::string, std::string> firstOrder = orderCollection[0];
    orderCollection.erase(orderCollection.begin());  
    return firstOrder;
  }
  std::map<std::string, std::string> emptyMap;
  return emptyMap;
}

void ClientSessionManager::pushSolution(std::string solution) {
  std::lock_guard<std::mutex> lock(mtx);
  solutionCollection.push_back(solution);
}

bool ClientSessionManager::isNumeric(const std::string& string) {
    static const std::regex numberRegex(
        R"(^[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?$)"
    );
    return std::regex_match(string, numberRegex);
}
