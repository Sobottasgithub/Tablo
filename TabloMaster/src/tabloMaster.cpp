#include <csignal>
#include <iostream>

#include "utils/network_manager.h"

int main(int argc, char *argv[])
{
    std::wcout << "Tablo Master" << std::endl;
    if (argc > 0) {
        for(int index = 0; index < argc; index++) {
            if (std::string(argv[index]).rfind("--interface", 0) == 0) {
                NetworkManager networkManager(argv[index+1]);
            }
        }
    } else {
        // Use config file to set Interface (TODO)
        NetworkManager networkManager("eth0");
    }

    return 0;
}

