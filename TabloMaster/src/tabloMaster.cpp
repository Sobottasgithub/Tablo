#include <csignal>
#include <iostream>

#include <networking.h>

#include "utils/network_manager.h"

int main(int argc, char *argv[])
{
    std::wcout << "Tablo Master" << std::endl;

    if (argc >= 2) {
        for(int index = 0; index < argc; index++) {
            if (std::string(argv[index]).rfind("--interface", 0) == 0) {
                std::string interface = argv[index+1];
                if (ttp2::Networking::isValidInterface(interface)) {
                    NetworkManager networkManager(interface);
                } else {
                    std::wcout << "Please provide a correct Interface!" << std::endl;
                }
            }
        }
    } else {
        std::wcout << "failed" << std::endl;
    }

    return 0;
}

