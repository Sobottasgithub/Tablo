#ifndef UDP_DISCOVERY_H
#define UDP_DISCOVERY_H

#include <string>
#include <vector>
#include <mutex>
#include <netinet/in.h>

class UdpDiscovery
{
    public:
        void udpDiscoveryCycle(std::string interface);
        
        std::vector<std::string> getNodeAdresses();
        void removeNodeAddress(std::string nodeAddress); 
        
    private:
        std::vector<std::string> nodeIPAddresses = {};
        std::mutex mtx;

        int sendMessageTo(int socket, const sockaddr_in& broadcast, std::string payload);
        std::string receiveMessage(int socket);
};

#endif
