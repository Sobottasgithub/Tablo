#include "client_session_controller.h"

#include <mutex>
#include <string>
#include <map>
#include <iostream>
#include <regex>
#include <cstring>
#include <sys/poll.h>
#include <sys/socket.h>

#include "methods.h"

using namespace std;

ClientSessionManager::ClientSessionManager() {}

ClientSessionManager::ClientSessionManager(int socket) {
  setSocket(socket);
}

void ClientSessionManager::setSocket(int socket) {
  this->socket = socket;
}

void ClientSessionManager::sessionControllerCycle() {
  while (true) {
    // Recieve orders
    std::string orderCountString = recieveMessage(this->socket);
    sendMessage(this->socket, std::to_string(Methods::success).c_str());
    if (isNumeric(orderCountString)) {
      int orderCount = std::stoi(orderCountString);  
      for (int index = 0; index < orderCount; index++) {
        std::string method  = recieveMessage(this->socket);
        sendMessage(this->socket, std::to_string(Methods::success).c_str());
        std::string content = recieveMessage(this->socket);
        sendMessage(this->socket, std::to_string(Methods::success).c_str());
        orderCollection.push_back({ {"method", method}, {"content", content} });
      }
    }

    // Send solutions
    do {
      sendMessage(this->socket, to_string(solutionCollection.size()).c_str());
    }
    while (std::stoi(recieveMessage(this->socket)) != Methods::success);
    for(int index = 0; index < solutionCollection.size(); index++) {
      do {
        sendMessage(this->socket, solutionCollection[0].c_str());
      }
      while (std::stoi(recieveMessage(this->socket)) != Methods::success);
      solutionCollection.erase(solutionCollection.begin());
    }
  }
}

bool ClientSessionManager::hasOrder() {
  std::lock_guard<std::mutex> lock(mtx);
  return !orderCollection.empty();
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

void ClientSessionManager::sendMessage(int socket, const char* initialMessage) {
    const char* message = initialMessage;
    send(socket, message, strlen(message), 0);
}

std::string ClientSessionManager::recieveMessage(int socket) {
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

bool ClientSessionManager::isNumeric(const std::string& s) {
    static const std::regex number_regex(
        R"(^[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?$)"
    );
    return std::regex_match(s, number_regex);
}
