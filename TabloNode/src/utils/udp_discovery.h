#ifndef UDP_DISCOVERY_H
#define UDP_DISCOVERY_H

#include <string>

class UdpDiscovery
{
    public:
        UdpDiscovery(std::string interface);

    private:
        std::string receiveMessage(int socket);
        int sendMessage(int socket, std::string payload);
};

#endif
