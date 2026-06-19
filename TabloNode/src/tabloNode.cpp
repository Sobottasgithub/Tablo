#include <iostream>

#include <networking.h>
#include <cctype>
#include <bits/stdc++.h>

#include "utils/network_manager.h"

int main(int argc, char *argv[])
{
    int maxConnections = 1000;
    std::wcout << "Tablo Node" << std::endl;
    if (argc >= 2) {
        std::string networkInterface = { "" };
        for(int index = 0; index < argc; index++) {
            if (std::string(argv[index]).rfind("--interface", 0) == 0) {
                std::string interface = argv[index+1];
                if (ttp2::Networking::isValidInterface(interface)) {
                    networkInterface = interface;
                } else {
                    std::wcout << "Please provide a correct Interface!" << std::endl;
                    return 1;
                }
            } else if (std::string(argv[index]).rfind("--maxConnections", 0) == 0 || std::string(argv[index]).rfind("-mc", 0) == 0) {
                char* maxConnectionsChar = argv[index+1];
                if (isdigit(*maxConnectionsChar)) {
                    maxConnections = atoi(maxConnectionsChar);
                }
            }
        }

        if (networkInterface.size() > 0) {
            NetworkManager networkManager(networkInterface, maxConnections);
        }
    } else {
        std::wcout << "failed" << std::endl;  
    }

    return 0;
}
