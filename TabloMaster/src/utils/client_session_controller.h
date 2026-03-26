#ifndef CLIENT_SESSION_CONTROLLER_H
#define CLIENT_SESSION_CONTROLLER_H

#include <string>
#include <vector>
#include <mutex>
#include "tabnet.h"

class ClientSessionManager
{
    public:
      ClientSessionManager();
      ClientSessionManager(int socket);
      void sessionControllerCycle();
      bool isConnected();
      bool hasOrder(); 
      tabnet::Packet popOrder();
      int getOrderCollectionSize();
      void pushSolution(tabnet::Packet); 
      void setSocket(int socket);

    private:       
      int socket;
      std::vector<tabnet::Packet> orderCollection;
      std::vector<tabnet::Packet> solutionCollection;
      bool connected = true;
      std::mutex mtx;
};

#endif
