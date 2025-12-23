#ifndef NETWORKING_H
#define NETWORKING_H

#include <string>

using namespace std;

class Networking
{
  public:
    Networking();
    Networking(std::string tabloMaster);

  private:
    std::string recieveMessage(int socket);
};

#endif
