#include "networking.h"

#include "tabnet.h"

#include "methods.h"
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
  int responseCode = 0;

  responseCode = tabnet::sendMessage(clientSocket, Methods::success, "");

  while (true) {
    tabnet::Packet data = tabnet::receiveMessage(clientSocket);
    if (data.method > Methods::START && data.method < Methods::END) {

      responseCode = tabnet::sendMessage(clientSocket, Methods::success, "");
      //tabnet::receiveMessage(clientSocket);

      std::string result = "";
      switch (data.method) {
      case Methods::test:
        result = worker.testCycle(data.payload);
        break;
      case Methods::setFile:
        std::wcout << "set file" << std::endl;
        result = "";
        break;
      }

      responseCode = tabnet::sendMessage(clientSocket, Methods::response, result.c_str());
      tabnet::Packet response = tabnet::receiveMessage(clientSocket);
    } else {
      // Bad request!
      responseCode = tabnet::sendMessage(clientSocket, Methods::failed, "Unexpected Method!");
    }

    // Implement controlled shutdown of this thread, if the master crashes
    if (responseCode < 0) {
      std::wcout << "Socket: " << clientSocket << " closed!" << std::endl;
      close(clientSocket);
      return;
    }
  }
}
