#include "node_session_controller.h"

#include "tabnet.h"
#include "methods.h"

#include <mutex>
#include <string>
#include <map>
#include <iostream>
#include <regex>
#include <cstring>
#include <sys/socket.h>
#include <thread>

NodeSessionController::NodeSessionController() {}

void NodeSessionController::sessionControllerCycle(int socket) {
  this->socket = socket;
  std::wcout << "CYCLE STARTED" << std::endl;

  int responseCode = 0;
  
  while (responseCode >= 0) {
    // Receive solutions
    tabnet::Packet solutionCount = tabnet::receiveMessage(socket);
    std::wcout << "solutionCount " << solutionCount.method << " payload " << solutionCount.payload.c_str() << std::endl;
    if (solutionCount.method == METHODS::size) {
      responseCode = tabnet::sendMessage(socket, METHODS::success, "");

      for (int index = 0; index < std::stoi(solutionCount.payload); index++) {
        tabnet::Packet packet = tabnet::receiveMessage(socket);
        std::wcout << "GOT SOLUTION -----> " << packet.method << std::endl;
        pushSolution(packet);
        responseCode = tabnet::sendMessage(socket, METHODS::success, "");
      }
    } else {
      responseCode = tabnet::sendMessage(socket, METHODS::failed, "");
      std::wcout << "Something went wrong during receiving size!" << std::endl;
      std::wcout << "Got: " << solutionCount.method << " instead of " << METHODS::size << " (size)" << std::endl;
    }

    tabnet::Packet ready = tabnet::receiveMessage(socket);
      
    // Send orders
    int orderCollectionSize = getOrderCollectionSize();
    responseCode = tabnet::sendMessage(socket, METHODS::size, std::to_string(orderCollectionSize));
    if (orderCollectionSize > 0) {
      if (tabnet::receiveMessage(socket).method == METHODS::success) {
        for(int index = 0; index < orderCollectionSize; index++) {
          responseCode = tabnet::sendPacket(socket, popOrder());
          tabnet::Packet response = tabnet::receiveMessage(socket);
          if (response.method != METHODS::success) {
            std::wcout << "Send order to node failed: got " << response.method << std::endl;
          }
        }
      } else {
        std::wcout << "Send of size failed!" << std::endl;
      }
    }

    if (responseCode < 0) {
        // TODO: FIX THIS
        responseCode = 0;
        //udpDiscovery.removeNodeAddress(nodeIps[index]);
        connected = false;
        break;
    }
  }
  connected = false;
}

bool NodeSessionController::hasSolution() {
  std::lock_guard<std::mutex> lock(mtx);
  return !solutionCollection.empty();
}

bool NodeSessionController::hasOrder() {
  std::lock_guard<std::mutex> lock(mtx);
  return !orderCollection.empty();
}

bool NodeSessionController::isConnected() {
  std::lock_guard<std::mutex> lock(mtx);
  return connected;
}

tabnet::Packet NodeSessionController::popOrder() {
  return popCollection(orderCollection);
}

tabnet::Packet NodeSessionController::popSolution() {
  return popCollection(solutionCollection);
}

tabnet::Packet NodeSessionController::popCollection(std::vector<tabnet::Packet> collection) {
  std::lock_guard<std::mutex> lock(mtx);
  if (!collection.empty()) {
    tabnet::Packet firstOrder = collection[0];
    collection.erase(collection.begin());  
    return firstOrder;
  }
  tabnet::Packet emptyPacket;
  return emptyPacket;
}

void NodeSessionController::pushSolution(tabnet::Packet solution) {
  std::lock_guard<std::mutex> lock(mtx);
  solutionCollection.push_back(solution);
}

void NodeSessionController::pushOrder(tabnet::Packet order) {
  std::lock_guard<std::mutex> lock(mtx);
  orderCollection.push_back(order);
}

int NodeSessionController::getOrderCollectionSize() {
  std::lock_guard<std::mutex> lock(mtx);
  return orderCollection.size();
}
