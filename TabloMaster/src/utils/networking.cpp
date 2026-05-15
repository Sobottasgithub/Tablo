// #include "udp_discovery.h"

#include "networking.h"

#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>

Networking::Networking(std::string interface) {
    std::wcout << "Start socket..." << std::endl;
    // udpDiscoveryThread = std::thread(&UdpDiscovery::udpDiscoveryCycle, &udpDiscovery, interface);


    // TODO: Implement accept client here
}

void Networking::handleClientConnection(int serverSocket, int clientSocket) {
    // TODO: add packet distribution logic here

    
    // udpDiscoveryThread.join();
}
