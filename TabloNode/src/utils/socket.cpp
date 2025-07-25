
#include "socket.h"

#include "worker.h"

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

Socket::Socket() {
    std::wcout << "Start Socket..." << endl;

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
    Socket::sendMessage(clientSocket, "100");

    struct pollfd pfds[2];
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;
    pfds[1].fd = clientSocket;
    pfds[1].events = POLLIN;

    while(poll(pfds, 2, 1000) != -1) {
        if(pfds[0].revents & POLLIN) {
            std::string message = Socket::recieveMessage(clientSocket);
            Socket::sendMessage(clientSocket, "200");
            worker.queTask(message);
            
            // send
            std::string returnVal = worker.getOutput();
            const char* result = returnVal.c_str();
            Socket::sendMessage(clientSocket, result);
        }
        if(pfds[1].revents & (POLLERR | POLLHUP)) {
            // close connection
            close(clientSocket);
            break;
        }
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

