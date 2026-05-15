#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <client_session_controller.h>
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <memory>

class NetworkManager
{
  public:
    NetworkManager();
    void networkingCycle(std::string tabloMaster);

    bool hasResponse();
    ClientSessionController::Packet popResponse();
    void pushRequest(ClientSessionController::Packet packet);

  private:
    std::shared_ptr<ClientSessionController> clientSessionController;
};

#endif
