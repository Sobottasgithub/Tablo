#ifndef SOCKET_H
#define SOCKET_H

#include <string> 
#include <vector>

class Socket
{
public:
    Socket();
    void sendMessage(int socket, const char* initialMessage);
    std::string recieveMessage(int socket);
    std::string get_broadcast_ip();
    std::string get_local_ip(); 
};

#endif
