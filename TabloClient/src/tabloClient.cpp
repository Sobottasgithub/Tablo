#include <iostream>
#include <string>
#include <cstring>

#include "utils/cli.h"

int main(int argc, char *argv[])
{
    if (argc > 0) {
        for(int index = 0; index < argc; index++) {
            if (std::string(argv[index]).rfind("--master", 0) == 0) {
                   Cli cli(argv[index+1]);
            }
        }
    } else {
        // Use config file (TODO)
        Cli cli("");
    }
    
    return 0;
}
