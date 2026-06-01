#include <iostream>
#include <string>
#include <cstring>

#include "utils/cli.h"
#include "utils/argv_struct.h"

int main(int argc, char *argv[])
{
    if (argc > 0) {
    struct Argv commandLineArguments;
        
        for(int index = 0; index < argc; index++) {
            if (std::string(argv[index]).rfind("--master", 0) == 0) {
                commandLineArguments.tabloMaster = argv[index+1];
            } else if (std::string(argv[index]).rfind("--file", 0) == 0) {
                commandLineArguments.filePath = argv[index+1];
            }
        }

        if (commandLineArguments.tabloMaster.length() != 0) {
            Cli cli(commandLineArguments);
        }
    } else {
        // Use config file (TODO)
        std::wcout << "failed" << std::endl;
    }
    
    return 0;
}
