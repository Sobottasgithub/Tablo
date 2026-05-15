#include "networking.h"

#include "tabnet.h"

#include "udp_discovery.h"
#include "worker.h"

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <ctime>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <system_error>
#include <thread>
#include <vector>

/*
Networking::Networking(std::string interface) {
  std::wcout << "Start Socket...." << std::endl;

  std::string containerIP = tabnet::getLocalIpAddress(interface);

  udpThread = std::thread(&Networking::handleUdpDiscovery, this, interface);

  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(4004);
  serverAddress.sin_addr.s_addr = inet_addr(containerIP.c_str());

  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));

  // On new request: make new worker thread
  threadCollection.reserve(100);
  listen(serverSocket, 5);
  while (true) {
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    std::wcout << "clientSocket: " << clientSocket << std::endl;
    threadCollection.push_back(std::thread(&Networking::handleClientConnection,
                                           this, serverSocket, clientSocket));
  }

  std::wcout << "Terminated!" << std::endl;

  for (auto &socketThread : threadCollection) {
    if (socketThread.joinable()) {
      socketThread.join();
    }
  }

  if (udpThread.joinable()) {
    udpThread.join();
  }
}

void Networking::handleUdpDiscovery(std::string interface) {
  UdpDiscovery udpDiscovery(interface);
}

void Networking::handleClientConnection(int serverSocket, int clientSocket) {
  Worker worker;
  std::thread workerThread = std::thread(&Worker::solveOrderCycle, &worker);

  // Compleate Handshake
  int responseCode = tabnet::sendMessage(clientSocket, METHODS::success, ""); 

  while (true) {
    // Hand back finished solution 
    int solutionCollectionSize = worker.getSolutionCollectionSize();
    responseCode = tabnet::sendMessage(clientSocket, METHODS::size, std::to_string(solutionCollectionSize));
    tabnet::Packet response = tabnet::receiveMessage(clientSocket);
    if (response.method == METHODS::success) {
      for(int index = 0; index < solutionCollectionSize; index++) {
        // Send data
        tabnet::Packet solution = worker.getSolution();
        responseCode = tabnet::sendPacket(clientSocket, solution);
        tabnet::Packet response = tabnet::receiveMessage(clientSocket);
        if (response.method != METHODS::success) {
          std::wcout << "Expected: " << METHODS::success << " (success) or " << METHODS::failed << " (failed), but got " << response.method << std::endl;
          std::wcout << "With following payload" << response.payload.c_str() << std::endl;
        }
      }
    } else if (response.method == METHODS::failed) {
      std::wcout << "Something went wrong while sending the size! Master response: " << response.payload.c_str() << std::endl;
    } else {
      std::wcout << "Expected: " << METHODS::success << " (success) or " << METHODS::failed << " (failed), but got " << response.method << std::endl;
      std::wcout << "With following payload" << response.payload.c_str() << std::endl;
    }

    responseCode = tabnet::sendMessage(clientSocket, METHODS::ready, "");
    
    //receive
    tabnet::Packet receivedPacket = tabnet::receiveMessage(clientSocket);
    if (receivedPacket.method == METHODS::size) {
      int count = std::stoi(receivedPacket.payload);
      if (count != 0) {
        responseCode = tabnet::sendMessage(clientSocket, METHODS::success, "");
        for(int i = 0; i < count; i++) {
          tabnet::Packet order = tabnet::receiveMessage(clientSocket);
          worker.pushOrder(order);
          responseCode = tabnet::sendMessage(clientSocket, METHODS::success, "");
        }
      }
    } else {
      std::wcout << "Expected: " << METHODS::size << " (size) got: " << receivedPacket.method << std::endl;
      responseCode = tabnet::sendMessage(clientSocket, METHODS::failed, "Expected method size");
    }

    // Controlled shutdown of this thread, if the master crashes
    if (responseCode < 0) {
      std::wcout << "Socket: " << clientSocket << " closed!" << std::endl;
      close(clientSocket);
      return;
    }    
  }
}
*/
