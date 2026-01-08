#ifndef CLIENT_SESSION_CONTROLLER_H
#define CLIENT_SESSION_CONTROLLER_H

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include "network_helpers.h"

using namespace std;

class ClientSessionManager
{
    public:
      ClientSessionManager();
      ClientSessionManager(int socket);
      void sessionControllerCycle();
      bool hasOrder(); 
      map<std::string, std::string> popOrder();
      void pushSolution(std::string solution); 
      void setSocket(int socket);

    private:
      NetworkHelpers networkHelpers;
       
      int socket;
      std::vector<std::map<std::string, std::string>> orderCollection;
      std::vector<std::string> solutionCollection;
      std::mutex mtx;

      bool isNumeric(const std::string& string);
};

#endif
