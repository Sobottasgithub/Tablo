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
      void pushSolution(std::string solution); 
      void setSocket(int socket);

    private:       
      int socket;
      std::vector<tabnet::Packet> orderCollection;
      std::vector<std::string> solutionCollection;
      bool connected = true;
      std::mutex mtx;

      bool isNumeric(const std::string& string);
};

#endif
