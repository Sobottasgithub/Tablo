
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <ctime>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <vector>

#include "socket.h"

#include "udp_discovery.h"

using namespace std;

Socket::Socket() {
    std::wcout << "Start tablo master socket..." << endl;

    UdpDiscovery udpDiscovery; 
    std::thread udpDiscoveryThread(&UdpDiscovery::udpDiscoveryCycle, &udpDiscovery);

    std::vector<std::string> nodeIps;

    while (true) {
        std::vector<std::string> newNodeIps = udpDiscovery.getNodeAdresses();

        if(newNodeIps.size() == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else {
            std::wcout << "Vector size: " << newNodeIps.size() << endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    
    udpDiscoveryThread.join();
    
    /*
    // creating socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // specifying address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("172.21.0.2");
    connect(clientSocket, (struct sockaddr*)&serverAddress,
    sizeof(serverAddress));

    struct pollfd pfds[2];
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;
    pfds[1].fd = clientSocket;
    pfds[1].events = POLLIN;
    
    std::string respCode;
    bool run = true;
    while(poll(pfds, 2, 60000) != -1 or run) {
        if(respCode == "100") {
            Socket::sendMessage(clientSocket, "Ping");
        }
        if(pfds[0].revents & POLLIN) {
            if(respCode == "100") {
                std::string respCode = Socket::recieveMessage(clientSocket);
                std::string msg = Socket::recieveMessage(clientSocket);
                cout << "Server Response: " << msg << " | with code: " << respCode << endl;
            } else {
                respCode = Socket::recieveMessage(clientSocket);
                cout << respCode << endl; 
            }
        }
        if(pfds[1].revents & (POLLERR | POLLHUP)) {
               cout << "TIMEOUT" << endl;
               break;
        }
    */
    
}

void Socket::sendMessage(int socket, const char* initialMessage) {
    const char* message = initialMessage;
    send(socket, message, strlen(message), 0);
}

std::string Socket::recieveMessage(int socket) {
    char buffer[1024] = { 0 };
    recv(socket, buffer, sizeof(buffer), 0);
    return buffer;
}

