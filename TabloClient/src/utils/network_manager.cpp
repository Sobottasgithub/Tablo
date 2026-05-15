#include "network_manager.h"

#include <client_session_controller.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <memory>
#include <type_traits>

NetworkManager::NetworkManager() {}

void NetworkManager::networkingCycle(std::string tabloMaster) {
    int serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(4003);
    serverAddress.sin_addr.s_addr = inet_addr(tabloMaster.c_str());

    int connectionResult = connect(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    
    if (connectionResult < 0 && errno != EINPROGRESS) {
        std::wcout << "Connection failed!" << std::endl;
        return;
    }
    
    clientSessionController = std::make_shared<ClientSessionController>(serverSocket);

    auto controller = clientSessionController;
    std::thread networkThread([controller]() {
      controller->networkingSession();
    });
}

bool NetworkManager::hasResponse() {
  auto controller = clientSessionController;
  return controller->hasResponse();
}

ClientSessionController::Packet NetworkManager::popResponse() {
  
}

void NetworkManager::pushRequest(ClientSessionController::Packet packet) {
  
}
