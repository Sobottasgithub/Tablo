#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

void sendMessage(int socket, const char* initialMessage) {
    // send initial request
    const char* message = initialMessage;
    send(socket, message, strlen(message), 0);
}

void recieveMessage(int socket) {
    // recieving data
    char buffer[1024] = { 0 };
    recv(socket, buffer, sizeof(buffer), 0);
    cout << "Answere from server: " << buffer
            << endl;
}

int main()
{
    // creating socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // specifying address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    connect(clientSocket, (struct sockaddr*)&serverAddress,
    sizeof(serverAddress));

    while(true) {
        // sending connection request

        sendMessage(clientSocket, "-HELLO12-");
        sendMessage(clientSocket, "-HELLO23-");

        recieveMessage(clientSocket);
        recieveMessage(clientSocket);
    }

    // closing socket
    close(clientSocket);

    return 0;
}