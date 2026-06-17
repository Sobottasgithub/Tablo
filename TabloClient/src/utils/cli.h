#ifndef CLI_H
#define CLI_H

#include "network_manager.h"

#include "argv_struct.h"

class Cli
{
  public:
    Cli(Argv* argv);

  private:
    void sendFile(std::string filePath, NetworkManager* networkManager);
};

#endif
