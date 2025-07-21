
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <ctime>

#include "socket.h"

using namespace std;

Socket::Socket() {
    // creating socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // specifying address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    connect(clientSocket, (struct sockaddr*)&serverAddress,
    sizeof(serverAddress));

    // TODO: Based on Diffie-Hellman key exchange: Agree on sharedKey
    //std::string clientKeyPart = generateKeyPart();

    //sendMessage(clientSocket, clientKeyPart.c_str());
    //std::this_thread::sleep_for(100ms);
    //std::string serverKeyPart = recieveMessage(clientSocket);

    //std::string sharedKey = clientKeyPart + serverKeyPart;

    //cout << "client: " << clientKeyPart << endl;
    //cout << "server: " << serverKeyPart << endl;
    //cout << "sharedKey: " << sharedKey << endl;

    while(true) {
        Socket::sendMessage(clientSocket, "HELLO");

        std::string respCode = Socket::recieveMessage(clientSocket);
        std::string msg = Socket::recieveMessage(clientSocket);
    }

    // closing socket
    close(clientSocket);
}

void Socket::sendMessage(int socket, const char* initialMessage) {
    // send initial request
    const char* message = initialMessage;
    send(socket, message, strlen(message), 0);
}

std::string Socket::recieveMessage(int socket) {
    // recieving data
    char buffer[1024] = { 0 };
    recv(socket, buffer, sizeof(buffer), 0);
    cout << "Answere from server: " << buffer
            << endl;
    return buffer;
}