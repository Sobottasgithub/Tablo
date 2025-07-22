#ifndef SOCKET_H
#define SOCKET_H

#include <string> 

class Socket
{
public:
    Socket();
    void sendMessage(int socket, const char* initialMessage);
    std::string recieveMessage(int socket);
};

#endif