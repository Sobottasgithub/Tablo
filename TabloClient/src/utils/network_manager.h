#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <client_session_controller.h>
#include <packet_types.h>
#include <string>
#include <memory>

class NetworkManager
{
  public:
    NetworkManager() = default;

    int createSocket(std::string tabloMaster);

    bool hasResponse();
    ttp2::Packet::Packet popResponse();
    void pushRequest(ttp2::Packet::Packet packet);

    bool isConnected();
    void disconnect();

  private:
    std::shared_ptr<ttp2::ClientSessionController> clientSessionController;
};

#endif
