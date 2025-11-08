
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

    // TCP
    std::vector<int> connections;
    std::vector<std::string> nodeIps;
    while (true) {
        std::vector<std::string> newNodeIps = udpDiscovery.getNodeAdresses();

        if(newNodeIps.size() == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            wcout << "No Orders can be compleated!" << endl;
        } else if(newNodeIps == nodeIps) {
            for(int index = 0; index < connections.size(); index++) {
                wcout << "--- START --- " << nodeIps[index].c_str() << " ("  << nodeIps.size() << ")" << endl;

                int currentSocket = connections[index]; 

                struct pollfd pfds[2];
                pfds[0].fd = STDIN_FILENO;
                pfds[0].events = POLLIN;
                pfds[1].fd = currentSocket;
                pfds[1].events = POLLIN;
            
                bool run = true;
                while(poll(pfds, 2, 60000) != -1 or run) {
                    
                    Socket::sendMessage(currentSocket, "Ping");
                    
                    if(pfds[0].revents & POLLIN) {
                        std::string respCode = Socket::recieveMessage(currentSocket);
                        std::string msg = Socket::recieveMessage(currentSocket);
                        std::wcout << "Server Response: " << msg.c_str() << " | with code: " << respCode.c_str() << " At ip: " << nodeIps[index].c_str() << endl;
                        break;
                    }
                    if(pfds[1].revents & (POLLERR | POLLHUP)) {
                           std::wcout << "TIMEOUT" << endl;
                           break;
                    }
                }

                std::wcout << "--- END --- " << nodeIps[index].c_str() << "\n" << endl;
            }

        } else if (newNodeIps != nodeIps) {
            wcout << "New nodes!" << endl;
            
            // Close previous connections if existing
            for (int index = 0; index < nodeIps.size(); index++) {
                wcout << "Close connection: " << nodeIps[index].c_str() << endl;
                close(connections[index]);
            }
            connections.clear();
            nodeIps.clear();

            // Establish new connections            
            nodeIps = newNodeIps;
            for (int index = 0; index < nodeIps.size(); index++) {
                // Create new connection
                int newSocket = socket(AF_INET, SOCK_STREAM, 0);
                
                sockaddr_in nodeAddress;
                nodeAddress.sin_family = AF_INET;
                nodeAddress.sin_port = htons(8080);
                nodeAddress.sin_addr.s_addr = inet_addr(nodeIps[index].c_str());
                connect(newSocket, (struct sockaddr*)&nodeAddress,
                sizeof(nodeAddress));

                struct pollfd pfds[2];
                pfds[0].fd = STDIN_FILENO;
                pfds[0].events = POLLIN;
                pfds[1].fd = newSocket;
                pfds[1].events = POLLIN;

                // Compleate handshake
                std::string respCode;
                while(poll(pfds, 2, 60000) != -1) {
                    if(pfds[0].revents & POLLIN) {
                        respCode = Socket::recieveMessage(newSocket);
                        std::wcout << "Connection established: " << respCode.c_str() << " at ip: " << nodeIps[index].c_str() << endl;
                        break;
                        
                    }
                    if(pfds[1].revents & (POLLERR | POLLHUP)) {
                           std::wcout << "TIMEOUT" << endl;
                           break;
                    }
                }

                // handshake compleate
                if(respCode == "100") {
                    connections.push_back(newSocket);
                }
            }

        } else {
            wcout << "unknown operation!" << endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

        }
    }
    
    udpDiscoveryThread.join();    
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

