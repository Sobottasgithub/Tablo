#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include "tabnet.h"

#include "utils/networking.h"

using namespace std;

int main(int argc, char *argv[])
{
    tabnet::test();
    std::wcout << "Tablo Node" << endl;
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
