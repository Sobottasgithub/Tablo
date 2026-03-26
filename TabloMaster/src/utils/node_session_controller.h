#ifndef NODE_SESSION_CONTROLLER_H
#define NODE_SESSION_CONTROLLER_H

#include <string>
#include <vector>
#include <mutex>
#include "tabnet.h"

class NodeSessionController
{
    public:
      NodeSessionController();
      void sessionControllerCycle(int socket);
      bool isConnected();
      bool hasOrder();
      bool hasSolution();
      tabnet::Packet popOrder();
      tabnet::Packet popSolution();
      int getOrderCollectionSize();
      void pushSolution(tabnet::Packet);
      void pushOrder(tabnet::Packet order); 
      
    private:       
      int socket;
      std::vector<tabnet::Packet> orderCollection;
      std::vector<tabnet::Packet> solutionCollection;
      bool connected = true;
      std::mutex mtx;
      tabnet::Packet popCollection(std::vector<tabnet::Packet> collection);
};

#endif
