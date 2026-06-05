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
    ttp2::ClientSessionController::Packet popResponse();
    void pushRequest(ttp2::ClientSessionController::Packet packet);

  private:
    std::shared_ptr<ttp2::ClientSessionController> clientSessionController;
};

#endif
