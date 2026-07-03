#include <iostream>

#include <networking.h>
#include <cctype>
#include <bits/stdc++.h>

#include "tablog.h"

#include "utils/network_manager.h"

int main(int argc, char *argv[])
{
    tablog::Tablog* logger = &tablog::Tablog::getInstance();
    logger->configure("Node", true);

    int maxConnections = 1000;
    if (argc >= 2) {
        std::string networkInterface = { "" };
        for(int index = 0; index < argc; index++) {
            if (std::string(argv[index]).rfind("--interface", 0) == 0) {
                std::string interface = argv[index+1];
                if (ttp2::Networking::isValidInterface(interface)) {
                    networkInterface = interface;
                } else {
                    logger->log(tablog::ERROR, "Please provide a correct network Interface");
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
        logger->log(tablog::ERROR, "Failed");  
    }

    return 0;
}
