#include "network_manager.h"

#include <server_session_controller.h>
#include <client_discovery.h>

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
#include <memory>

NetworkManager::NetworkManager(std::string interface) {
  std::wcout << "Start Socket...." << std::endl;
  ttp2::ServerSessionController serverSessionController;
  
  std::string containerIP = serverSessionController.getLocalIpAddress(interface);

  auto clientDiscovery = std::make_shared<tud::ClientDiscovery>(interface, 4000, 4001, "Tablo");
  std::thread udpThread([clientDiscovery]() {
    clientDiscovery->discoveryCycle();
  });

  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(4004);
  serverAddress.sin_addr.s_addr = inet_addr(containerIP.c_str());

  int serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if(bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
     std::wcout << "Bind failed!" << std::endl;
     return;
  }

  // Create epoll
  int epollFd = epoll_create1(0);
  if (epollFd == -1) {
      std::wcout << "Failed to create epoll!" << std::endl;
  }
  // Set epoll action for server
  struct epoll_event serverEvents;
  serverEvents.events = EPOLLIN;
  serverEvents.data.fd = serverSocket;
  if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &serverEvents) == -1) {
      std::wcout << "Failed to set epoll_ctl!" << std::endl;
      return;
  }

  listen(serverSocket, 5);
  std::vector<std::thread> clientConnections;
  while (true) {
      const int MAX_EVENTS = 10;
      struct epoll_event events[MAX_EVENTS];
      int epollRequestCount = epoll_wait(epollFd, events, MAX_EVENTS, -1);
    
      for (int index = 0; index < epollRequestCount; ++index) {
          if (events[index].data.fd == serverSocket) {
              int clientSocket = accept4(serverSocket, nullptr, nullptr, SOCK_NONBLOCK);
              std::wcout << "New clientSocket: " << clientSocket << std::endl;

              clientConnections.push_back(std::thread([this, serverSocket, clientSocket]() {
                    this->handleClientConnection(serverSocket, clientSocket);
              }));
          }
      }
  }

  std::wcout << "Terminated!" << std::endl;

  for (auto &socketThread : clientConnections) {
    if (socketThread.joinable()) {
      socketThread.join();
    }
  }

  if (udpThread.joinable()) {
    udpThread.join();
  }
}

void NetworkManager::handleClientConnection(int serverSocket, int clientSocket) {
  auto serverSessionController = std::make_shared<ttp2::ServerSessionController>(serverSocket, clientSocket);

  std::thread networkingSession([serverSessionController]() {
      serverSessionController->networkingSession();
  });

  Worker worker;
  std::thread workerThread = std::thread(&Worker::solveRequestCycle, &worker);

  while (serverSessionController->isConnected()) {
    if (serverSessionController->hasRequest()) {
     std::wcout << "Received Request!" << std::endl;
     worker.pushRequest(serverSessionController->popRequest());
    }

    while (worker.getResponseCollectionSize() > 0) {
      std::wcout << "Hand back Response" << std::endl;
      serverSessionController->pushResponse(worker.getResponse());
    }
  }
  if (workerThread.joinable()) {
    workerThread.join();
  }
}

