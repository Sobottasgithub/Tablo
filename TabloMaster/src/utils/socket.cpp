
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
#include <algorithm>
#include <map>

#include "socket.h"
#include "methods.h"

#include "udp_discovery.h"

using namespace std;

Socket::Socket() {
    std::wcout << "Start tablo master socket..." << endl;

    std::string method = std::to_string(Methods::test).c_str();
    std::string data = "test";

    UdpDiscovery udpDiscovery; 
    std::thread udpDiscoveryThread(&UdpDiscovery::udpDiscoveryCycle, &udpDiscovery);

    // TCP
    std::map<std::string, int> connections;
    while (true) {
        std::vector<std::string> newNodeIps = udpDiscovery.getNodeAdresses();
        std::vector<std::string> nodeIps = Socket::getKeys(connections);

        // sort vectors to compare them
        std::sort(newNodeIps.begin(), newNodeIps.end());
        std::sort(nodeIps.begin(), nodeIps.end());

        if(newNodeIps.size() == 0) {
            wcout << "No connected nodes!" << endl;
            if (nodeIps.size() > 0) {
                // Close connections with nodes that dont exist anymore
                for (int index = 0; index < nodeIps.size(); index++) {
                    std::wcout << "Close connection: " << nodeIps[index].c_str() << endl;
                    close(connections[nodeIps[index]]);
                    connections.erase(nodeIps[index]);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else if(newNodeIps == nodeIps) {
            for(int index = 0; index < connections.size(); index++) {

                int currentSocket = connections[nodeIps[index]];
                
                Socket::sendMessage(currentSocket, method.c_str());
                std::string status = Socket::recieveMessage(currentSocket);
                if (!status.empty() && std::all_of(status.begin(), status.end(), ::isdigit) && std::stoi(status) == Methods::success) {
                    Socket::sendMessage(currentSocket, data.c_str());
                    std::string status = Socket::recieveMessage(currentSocket);
                    if (!status.empty() && std::all_of(status.begin(), status.end(), ::isdigit) && std::stoi(status) == Methods::success) {
                        std::string recievedData = Socket::recieveMessage(currentSocket);
                        Socket::sendMessage(currentSocket, std::to_string(Methods::success).c_str());
                        wcout << "Response: " << recievedData.c_str() << " | Node: " << nodeIps[index].c_str() << endl;
                    } else {
                        wcout << "Method " << method.c_str() << " failed with status: " << status.c_str() << endl;
                    }
                } else {
                    wcout << "Method " << method.c_str() << " failed with status: " << status.c_str() << endl;
                }
            }
        } else if (newNodeIps != nodeIps) {
            wcout << "New nodes!" << endl;

            // Create connections with nodes
            for (int index = 0; index < newNodeIps.size(); index++) {
                // If newNodeIps[index] not in nodeIps (keys of connections)
                if(std::find(nodeIps.begin(), nodeIps.end(), newNodeIps[index]) == nodeIps.end()) {
                    // Create new connection
                    int newSocket = socket(AF_INET, SOCK_STREAM, 0);
            
                    sockaddr_in nodeAddress;
                    nodeAddress.sin_family = AF_INET;
                    nodeAddress.sin_port = htons(8080);
                    nodeAddress.sin_addr.s_addr = inet_addr(newNodeIps[index].c_str());
                    connect(newSocket, (struct sockaddr*) &nodeAddress, sizeof(nodeAddress));

                    std::string respCode = Socket::recieveMessage(newSocket);

                    // handshake compleate
                    if(!respCode.empty() && std::all_of(respCode.begin(), respCode.end(), ::isdigit) && std::stoi(respCode) == Methods::success) {
                        connections.insert({newNodeIps[index], newSocket});
                        std::wcout << "Connection established: " << respCode.c_str() << " at ip: " << newNodeIps[index].c_str() << endl;
                    }
                }
            }
                        
            // Close connections with nodes that dont exist anymore
            for (int index = 0; index < nodeIps.size(); index++) {
                if(std::find(newNodeIps.begin(), newNodeIps.end(), nodeIps[index]) == newNodeIps.end()) {
                    std::wcout << "Close connection: " << nodeIps[index].c_str() << endl;
                    close(connections[nodeIps[index]]);
                    connections.erase(nodeIps[index]);
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
    struct pollfd pfds[2];
    pfds[0] = pollfd {
        .fd = STDIN_FILENO,
        .events = POLLIN
    };

    pfds[1] = pollfd {
        .fd = socket,
        .events = POLLIN
    };

    // Compleate handshake
    std::string respCode;
    while(poll(pfds, 2, 60000) != -1) {
        if(pfds[0].revents & POLLIN) {
            char buffer[1024] = { 0 };
            recv(socket, buffer, sizeof(buffer), 0);
            return buffer;
        }
        if(pfds[1].revents & (POLLERR | POLLHUP)) {
            wcout << "TIMEOUT" << endl;
            return "";
        }
    }
}

std::vector<std::string> Socket::getKeys(std::map<std::string, int> hashmap) {
    std::vector<std::string> keys;
    // preallocate memory of exact size
    keys.reserve(hashmap.size());

    for (const auto& pair : hashmap) {
        keys.push_back(pair.first);
    }

    return keys;
}
