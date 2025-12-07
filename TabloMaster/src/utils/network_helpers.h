#ifndef NETWORK_HELPERS_H
#define NETWORK_HELPERS_H

#include <string>

class NetworkHelpers
{
    public:
        std::string getLocalIpAddress(std::string interface);
        std::string getBroadcastIpAddress(); 
};

#endif
