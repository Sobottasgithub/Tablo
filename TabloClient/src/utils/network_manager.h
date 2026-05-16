#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <client_session_controller.h>
#include <string>
#include <thread>
#include <mutex>
#include <memory>

class NetworkManager
{
  public:
    NetworkManager();

    int createSocket(std::string tabloMaster);

    bool hasResponse();
    ClientSessionController::Packet popResponse();
    void pushRequest(ClientSessionController::Packet packet);

  private:
    std::shared_ptr<ClientSessionController> clientSessionController;
};

#endif
