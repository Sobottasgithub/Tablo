
#include "udp_discovery.h"

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

using namespace std;

UdpDiscovery::UdpDiscovery() {
    std::wcout << "Start udp discovery..." << endl;
    
    int udpSocket;
    struct sockaddr_in nodeAddress{};
    const int port = 8888;
    char buffer[1024];

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    int reuse = 1;
    setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    nodeAddress.sin_family = AF_INET;
    nodeAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    nodeAddress.sin_port = htons(port);

    bind(udpSocket, (struct sockaddr*)&nodeAddress, sizeof(nodeAddress));

    std::wcout << "Listening for UDP broadcast... port: " << port << endl;

    while (true) {
        sockaddr_in masterAddress{};
        socklen_t masterAdressLength = sizeof(masterAddress);

        ssize_t recvLen = recvfrom(udpSocket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&masterAddress, &masterAdressLength);
        buffer[recvLen] = '\0';

        char masterIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &masterAddress.sin_addr, masterIp, INET_ADDRSTRLEN);

        std::string reply = "102";
        if (sendto(udpSocket, reply.c_str(), reply.length(), 0,
                   (struct sockaddr*)&masterAddress, masterAdressLength) < 0) {
            std::wcout << "Reply failed to: " << masterIp << endl;
        }
    }

    close(udpSocket);
}

