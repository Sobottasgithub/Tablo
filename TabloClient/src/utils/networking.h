#ifndef NETWORKING_H
#define NETWORKING_H

#include <client_session_controller.h>
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <memory>

class Networking
{
  public:
    Networking();
    void networkingCycle(std::string tabloMaster);

    bool hasResponse();
    ClientSessionController::Packet popResponse();
    void pushRequest(ClientSessionController::Packet packet);

  private:
    std::shared_ptr<ClientSessionController> clientSessionController;
};

#endif
