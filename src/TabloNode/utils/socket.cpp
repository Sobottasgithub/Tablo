
#include "socket.h"

#include "worker.h"

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <bits/stdc++.h>
#include <vector>
#include <ctime>

using namespace std;

Socket::Socket() {
    std::wcout << "Start Socket..." << endl;

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY; //inet_addr("127.0.0.1")

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // On new request: make new worker thread
    threadCollection.reserve(100);
    listen(serverSocket, 5);
    while(true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        std::wcout << "clientSocket: " << clientSocket << endl;
        threadCollection.push_back(std::thread(
            &Socket::handleClientConnection,
            this,
            serverSocket, clientSocket
        ));
    }

    for (auto &socketThread : threadCollection) {
        if(socketThread.joinable()) {
            socketThread.join();
        }
    }
}

void Socket::handleClientConnection(int serverSocket, int clientSocket) {
    Worker worker;

    // Based on Diffie-Hellman key exchange: Agree on sharedKey
    //std::string serverKeyPart = generateKey();
    //std::string clientKeyPart = Socket::recieveMessage(clientSocket);
    //Socket::sendMessage(clientSocket, serverKeyPart.c_str());
    //std::string sharedKey = clientKeyPart + serverKeyPart;

    while(true) {
        // recieve
        std::string message = Socket::recieveMessage(clientSocket);
        Socket::sendMessage(clientSocket, "200");
        worker.queTask(message);

        // send
        std::string returnVal = worker.getOutput();
        const char* result = returnVal.c_str();
        std::this_thread::sleep_for(100ms);
        Socket::sendMessage(clientSocket, result);
    }
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

