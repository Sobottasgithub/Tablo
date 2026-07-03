#include <iostream>

#include <networking.h>

#include "utils/network_manager.h"

#include "tablog.h"

int main(int argc, char *argv[])
{
    tablog::Tablog* logger = &tablog::Tablog::getInstance();
    logger->configure("Master", true);

    if (argc >= 2) {
        for(int index = 0; index < argc; index++) {
            if (std::string(argv[index]).rfind("--interface", 0) == 0) {
                std::string interface = argv[index+1];
                if (ttp2::Networking::isValidInterface(interface)) {
                    NetworkManager networkManager(interface);
                } else {
                    logger->log(tablog::ERROR, "Please provide a correct network Interface");
                }
            }
        }
    } else {
        std::wcout << "failed" << std::endl;
    }

    return 0;
}

