#include "networking.h"

#include "worker.h"
#include "methods.h"
#include "udp_discovery.h"

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <system_error>
#include <thread>
#include <bits/stdc++.h>
#include <vector>
#include <ctime>
#include <arpa/inet.h>

using namespace std;

Networking::Networking(std::string interface) {
    std::wcout << "Start Socket...." << endl;

    NetworkHelpers networkHelpers;
    std::string containerIP = networkHelpers.getLocalIpAddress(interface);

    udpThread = std::thread(
            &Networking::handleUdpDiscovery,
            this,
            interface
    );

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(4004);
    serverAddress.sin_addr.s_addr = inet_addr(containerIP.c_str());

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // On new request: make new worker thread
    threadCollection.reserve(100);
    listen(serverSocket, 5);
    while(true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        std::wcout << "clientSocket: " << clientSocket << endl;
        threadCollection.push_back(std::thread(
            &Networking::handleClientConnection,
            this,
            serverSocket, clientSocket
        )); 
    }

    wcout << "Terminated!" << endl;
    
    for (auto &socketThread : threadCollection) {
        if(socketThread.joinable()) {
            socketThread.join();
        }
    }

    if(udpThread.joinable()) {
        udpThread.join();
    }   
}

void Networking::handleUdpDiscovery(std::string interface) {
    UdpDiscovery udpDiscovery(interface);    
}

void Networking::handleClientConnection(int serverSocket, int clientSocket) {
    Worker worker;

    networkHelpers.sendMessage(clientSocket, std::to_string(Methods::success).c_str());

    while (true) {
        std::string methodString = networkHelpers.receiveMessage(clientSocket);

        // Check if string is numeric
        if (!methodString.empty() && std::all_of(methodString.begin(), methodString.end(), ::isdigit)) {
            // Convert methodString to int
            int method = std::stoi( methodString );

            if(method > Methods::START && method < Methods::END) {
                // Valid method: success            
                networkHelpers.sendMessage(clientSocket, std::to_string(Methods::success).c_str());
            
                std::string data = networkHelpers.receiveMessage(clientSocket);

                // got data
                networkHelpers.sendMessage(clientSocket, std::to_string(Methods::success).c_str());

                std::string result = "";
                switch (method) {
                    case Methods::test:
                        result = worker.testCycle(data);
                        break;
                    case Methods::setFile:
                        wcout << "set file" << endl;
                        result = "";
                        break;
                }                

                networkHelpers.sendMessage(clientSocket, result.c_str());
                std::string response = networkHelpers.receiveMessage(clientSocket);                
            } else {
                // Bad request!
                networkHelpers.sendMessage(clientSocket, std::to_string(Methods::failed).c_str());
            }
        } else {
            // Method not found. Bad request! Failed!
            networkHelpers.sendMessage(clientSocket, std::to_string(Methods::failed).c_str());
        }
    }
}
