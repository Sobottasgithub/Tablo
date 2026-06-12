#include "network_manager.h"

#include <client_session_controller.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <memory>
#include <cerrno>
#include <type_traits>

NetworkManager::NetworkManager() {}

int NetworkManager::createSocket(std::string tabloMaster) {
    int serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(4003);
    serverAddress.sin_addr.s_addr = inet_addr(tabloMaster.c_str());

    int connectionResult = connect(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));

    // Wait for server to accept
    if (connectionResult < 0 && errno != EINPROGRESS) {
        std::wcout << "Connection failed!" << std::endl;
        return -1;
    }

    clientSessionController = std::make_shared<ttp2::ClientSessionController>(serverSocket);

    std::thread networkThread([this]() {
        clientSessionController->networkingSession();
    });
    networkThread.detach();

    return 0;
}

bool NetworkManager::hasResponse() {
  return clientSessionController->hasResponse();
}

ttp2::ClientSessionController::Packet NetworkManager::popResponse() {
  return clientSessionController->popResponse();
}

void NetworkManager::pushRequest(ttp2::Networking::Packet packet) {
  clientSessionController->pushRequest(packet);
}
