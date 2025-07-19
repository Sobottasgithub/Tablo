#ifndef SOCKET_H
#define SOCKET_H

#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <thread>

#include <bits/stdc++.h>


using namespace std::chrono_literals;
using namespace std;

class Socket {

    sockaddr_in serverAddress;
    int serverSocket;
    int clientSocket;

    bool socketOnline = true;
    std::thread thread;

    vector<string> inputQue;
    vector<string> outputQue;

    public:
        Socket(int port){

            // specifying the address
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_port = htons(port);
            serverAddress.sin_addr.s_addr = INADDR_ANY; //inet_addr("127.0.0.1")

            thread = std::thread(&Socket::mainLoop, this);

            //TODO: CHECK FOR NEW METHOD TO PULL ELSE GO ON
        }

        void mainLoop() {
            cout << "new Loop" << endl;
            int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

            bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

            while(socketOnline) {
                listen(serverSocket, 5);
                
                
                //if(pollMessage(clientSocket)){
                // accepting connection request
                int clientSocket = accept(serverSocket, nullptr, nullptr);

                if(pollMessage(clientSocket)){
                    string buffer = recieveMessage(clientSocket);
                    message = buffer;
                }

                sendMessage(clientSocket, "200");

                string outPutString; {
                    std::lock_guard<std::mutex> lock(message_mutex);
                    outPutString = outputMessage;
                }

                if(outPutString != "") {
                    sendMessage(clientSocket, outPutString);
                }
                
                cout << "DONE" << endl;
            }
        }

        bool pollMessage(int socket) {
            cout << "POLL MESSAGE" << endl;
            int    selectStatus;                                 /* select() return code */
            char     tempreport[ 256 ] = {'\0'};
            struct   timeval tv;
            fd_set   fdread;

            do{
                tv.tv_sec = 1;
                tv.tv_usec = 0;

                FD_ZERO(&fdread);
                //FD_SET( FD, &fdread );

                selectStatus = select(socket+1, &fdread, NULL, NULL, &tv);
                cout << "STATUs: " << selectStatus << endl;

                switch(selectStatus)
                {
                    case -1:
                        cout << "No MESSAGE" << endl;
                        return false;

                    case 0:
                        cout << "Is MESSAGE" << endl;
                        return true;

                    default:
                        cout << "default MESSAGE" << endl;
                        memset(tempreport, 0x00, sizeof(tempreport));
                        return false;
                }
            } while(0 < selectStatus);
        }

        string getInput() {
            std::lock_guard<std::mutex> lock(message_mutex);
            return message;
        }

        void setOutput(string output) {
            std::lock_guard<std::mutex> lock(message_mutex);
            outputMessage = output;
        }


        void join() {
            if (thread.joinable()) {
                thread.join();
            }
        }

        void detatch() {
            thread.detach();
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

        void terminate() {
            bool socketOnline = false;
            close(serverSocket);
        }

    private:
        bool firstConnect = true;
        string message = "";
        string outputMessage;
        mutable std::mutex message_mutex;
};

#endif