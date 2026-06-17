#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>

#include <networking.h>

#include "utils/cli.h"
#include "utils/argv_struct.h"

int main(int argc, char *argv[])
{
    if (argc > 0) {
        Argv commandLineArguments;
        
        for(int index = 0; index < argc; index++) {
            if (std::string(argv[index]).rfind("--master", 0) == 0) {
                std::string masterIp = argv[index+1];
                if (ttp2::Networking::isValidIpV4(masterIp)) {
                    commandLineArguments.tabloMaster = masterIp;
                } else {
                    std::wcout << "Please provide a correct IPv4!" << std::endl;
                }
            } else if (std::string(argv[index]).rfind("--file", 0) == 0) {
                std::string filepath = argv[index+1];
                if (std::filesystem::exists(filepath)) {
                    commandLineArguments.filePath = argv[index+1];
                } else {
                    std::wcout << "Please provide a correct Filepath" << std::endl;
                }
            }
        }

        if (commandLineArguments.tabloMaster.length() != 0) {
            Cli cli(&commandLineArguments);
        }
    } else {
        // Use config file (TODO)
        std::wcout << "failed" << std::endl;
    }
    
    return 0;
}
