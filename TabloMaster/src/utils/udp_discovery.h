#ifndef UDP_DISCOVERY_H
#define UDP_DISCOVERY_H

#include <string>

class UdpDiscovery
{
    public:
        UdpDiscovery();
    private:
        std::string getLocalIpAddress();
        std::string getBroadcastIpAddress(); 
};

#endif
