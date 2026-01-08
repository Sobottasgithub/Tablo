#include <iostream>
#include <string>
#include <cstring>

#include "utils/networking.h"

int main(int argc, char *argv[])
{
    std::wcout << "Tablo Client" << std::endl;
    if (argc > 0) {
        for(int index = 0; index < argc; index++) {
            if (std::string(argv[index]).rfind("--master", 0) == 0) {
                   Networking networking(argv[index+1]);
            }
        }
    } else {
        // Use config file (TODO)
        Networking networking;
    }
    
    return 0;
}
