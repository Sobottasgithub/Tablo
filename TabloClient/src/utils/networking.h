#ifndef NETWORKING_H
#define NETWORKING_H

#include <string>
#include <map>
#include <vector>
#include <mutex>

class Networking
{
  public:
    Networking();
    void networkingCycle(std::string tabloMaster);

    bool hasSolution();
    std::string popSolution();
    void pushOrder(int method, std::string content); 

  private:
    std::vector<std::map<std::string, std::string>> orderCollection;
    std::vector<std::string> solutionCollection;
    std::mutex mtx;

    std::string recieveMessage(int socket);
    bool isNumeric(const std::string& string);
};

#endif
