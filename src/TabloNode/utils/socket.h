#ifndef SOCKET_H
#define SOCKET_H

#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;
using namespace std;

class Socket {
    int serverSocket;
    int clientSocket;
    bool socketOnline = true;
    public:
        Socket(){
            // creating socket
            int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

            // specifying the address
            sockaddr_in serverAddress;
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_port = htons(8080);
            serverAddress.sin_addr.s_addr = INADDR_ANY;

            // binding socket.
            bind(serverSocket, (struct sockaddr*)&serverAddress,
                    sizeof(serverAddress));

            // listening to the assigned socket
            listen(serverSocket, 5);

            // accepting connection request
            int clientSocket = accept(serverSocket, nullptr, nullptr);

            string buffer = recieveMessage(clientSocket);
            string bufferClone = buffer;

            sendMessage(clientSocket, buffer);
            

            // closing the socket.
            close(serverSocket);
        }
        
        void sendMessage(int socket, string initialMessage) {
            const char * message = initialMessage.c_str();
            send(socket, message, strlen(message), 0);
        }
        
        std::string recieveMessage(int socket) {
                // recieving data
                char buffer[1024] = { 0 };
                recv(socket, buffer, sizeof(buffer), 0);
                cout << "Message from client: " << buffer
                        << endl;
                
                return buffer;
        }

        std::string pollMessage() {
            int    selectStatus;                                 /* select() return code */
            char     tempreport[ 256 ] = {'\0'};
            struct   timeval tv;
            fd_set   fdread;

            do{
                tv.tv_sec = 1;
                tv.tv_usec = 0;

                FD_ZERO(&fdread);
                //FD_SET( FD, &fdread );

                selectStatus = select(clientSocket+1, &fdread, NULL, NULL, &tv);

                switch(selectStatus)
                {
                    case -1:
                        return "Other error";

                    case 0:
                        return "Timeout";

                    default:
                        memset(tempreport, 0x00, sizeof(tempreport));
                        return "";
                }
            } while(0 < selectStatus);
        }
};

#endif