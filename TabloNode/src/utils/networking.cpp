
#include "networking.h"

#include "worker.h"
#include "methods.h"
#include "udp_discovery.h"

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <bits/stdc++.h>
#include <vector>
#include <ctime>
#include <arpa/inet.h>
#include <sys/poll.h>


using namespace std;

Networking::Networking() {
    std::wcout << "Start Socket...." << endl;

    udpThread = std::thread(
            &Networking::handleUdpDiscovery,
            this
    );

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("0.0.0.0"); //inet_addr("127.0.0.1")

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

void Networking::handleUdpDiscovery() {
    UdpDiscovery udpDiscovery;    
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
    struct pollfd pfds[2];
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;
    pfds[1].fd = socket;
    pfds[1].events = POLLIN;

    while(poll(pfds, 2, 1000) != -1) {
        if(pfds[0].revents & POLLIN) {
            char buffer[1024] = { 0 };
            recv(socket, buffer, sizeof(buffer), 0);
            return buffer;
        }
        if(pfds[1].revents & (POLLERR | POLLHUP)) {
            // close connection
            std::wcout << "TIMEOUT" << endl;
            close(socket);
            return "";
        }
    }
}

