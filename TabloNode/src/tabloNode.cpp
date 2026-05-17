#include <csignal>
#include <iostream>

//#include "utils/networking.h"
#include "utils/udp_discovery.h"

int main(int argc, char *argv[])
{
    // Prevents crash when tcp send failes
    signal(SIGPIPE, SIG_IGN);
    
    std::wcout << "Tablo Node" << std::endl;
    UdpDiscovery udpDiscovery("enp39s0");
    // if (argc > 0) {
    //     for(int index = 0; index < argc; index++) {
    //         if (std::string(argv[index]).rfind("--interface", 0) == 0) {
    //             //Networking networking(argv[index+1]);
    //         }
    //     }
    // } else {
    //     // Use config file to set Interface (TODO)
    //     //Networking networking("eth0");
    //     UdpDiscovery udpDiscovery("enp39s0");
    // }

    return 0;
}
