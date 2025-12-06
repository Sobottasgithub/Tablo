#ifndef UDP_DISCOVERY_H
#define UDP_DISCOVERY_H

#include <string>
#include <vector>
#include <mutex>

class UdpDiscovery
{
    public:
        void udpDiscoveryCycle();
        std::vector<std::string> getNodeAdresses();
        
    private:
        std::vector<std::string> nodeIPAddresses = {};
        std::mutex mtx;
};

#endif
