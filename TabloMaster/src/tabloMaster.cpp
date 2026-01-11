#include <csignal>
#include <iostream>

#include "utils/networking.h"

int main(int argc, char *argv[])
{
    // Prevents crash when tcp send failes
    signal(SIGPIPE, SIG_IGN);

    std::wcout << "Tablo Master" << std::endl;
    if (argc > 0) {
        for(int index = 0; index < argc; index++) {
            if (std::string(argv[index]).rfind("--interface", 0) == 0) {
                Networking networking(argv[index+1]);
            }
        }
    } else {
        // Use config file to set Interface (TODO)
        Networking networking("eth0");
    }

    return 0;
}

