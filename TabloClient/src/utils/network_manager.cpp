#include "network_manager.h"

#include <client_session_controller.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <memory>
#include <cerrno>
#include <type_traits>
#include <poll.h>

#include "tablog.h"

int NetworkManager::createSocket(std::string tabloMaster) {
    tablog::Tablog* logger = &tablog::Tablog::getInstance();

    int serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(4003);
    serverAddress.sin_addr.s_addr = inet_addr(tabloMaster.c_str());

    int connectionResult = connect(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));

    // Wait for server to accept
    if (connectionResult < 0) {
        if (errno == EINPROGRESS) {
            struct pollfd pfd;
            pfd.fd = serverSocket;
            pfd.events = POLLOUT;

            // Wait max 10 Seconds for connection
            int pollResult = poll(&pfd, 1, 10000);

            if (pollResult > 0) {
                int socketError = 0;
                socklen_t len = sizeof(socketError);
                getsockopt(serverSocket, SOL_SOCKET, SO_ERROR, &socketError, &len);

                if (socketError != 0) {
                    logger->log(tablog::ERROR, "Connection failed!");
                    return -1; 
                }
            } else {
              logger->log(tablog::ERROR, "Connection failed!");
              return -1;
            }
        } else {
          logger->log(tablog::ERROR, "Connection failed!");
          return -1;
        }
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
