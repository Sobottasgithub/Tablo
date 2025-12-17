
#include "networking.h"

#include "worker.h"
#include "methods.h"
#include "udp_discovery.h"
#include "network_helpers.h"

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <system_error>
#include <thread>
#include <bits/stdc++.h>
#include <vector>
#include <ctime>
#include <arpa/inet.h>
#include <sys/poll.h>


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
    serverAddress.sin_port = htons(8000);
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

    Networking::sendMessage(clientSocket, std::to_string(Methods::success).c_str());

    while (true) {
        std::string methodString = Networking::recieveMessage(clientSocket);

        // Check if string is numeric
        if (!methodString.empty() && std::all_of(methodString.begin(), methodString.end(), ::isdigit)) {
            // Convert methodString to int
            int method = std::stoi( methodString );

            if(method > Methods::START && method < Methods::END) {
                // Valid method: success            
                Networking::sendMessage(clientSocket, std::to_string(Methods::success).c_str());
            
                std::string data = Networking::recieveMessage(clientSocket);

                // got data
                Networking::sendMessage(clientSocket, std::to_string(Methods::success).c_str());

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

                Networking::sendMessage(clientSocket, result.c_str());
                std::string response = Networking::recieveMessage(clientSocket);                
            } else {
                // Bad request!
                Networking::sendMessage(clientSocket, std::to_string(Methods::failed).c_str());
            }
        } else {
            // Method not found. Bad request! Failed!
            Networking::sendMessage(clientSocket, std::to_string(Methods::failed).c_str());
        }
    }
}

void Networking::sendMessage(int socket, const char* initialMessage) {
    const char* message = initialMessage;
    send(socket, message, strlen(message), 0);
}


std::string Networking::recieveMessage(int socket) {
    pollfd pfd{};
    pfd.fd = socket;
    pfd.events = POLLIN;

    int ret = poll(&pfd, 1, 10000);
    if (ret > 0 && (pfd.revents & POLLIN)) {
        char buffer[1024]{};
        ssize_t n = recv(socket, buffer, sizeof(buffer)-1, 0);
        if (n <= 0) return "";
        return std::string(buffer, n);
    }
    return "";
}

