#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>
#include <mutex>

#include <networking.h>

#include "tablog.h"

#include "utils/cli.h"
#include "utils/argv_struct.h"

int main(int argc, char *argv[])
{

    tablog::Tablog* logger = &tablog::Tablog::getInstance();
    logger->configure("Client", true);

    if (argc >= 2) {
        Argv commandLineArguments;
        
        for(int index = 0; index < argc; index++) {
            if (std::string(argv[index]).rfind("--master", 0) == 0) {
                std::string masterIp = argv[index+1];
                if (ttp2::Networking::isValidIpV4(masterIp)) {
                    commandLineArguments.tabloMasterIp = masterIp;
                } else {
                    logger->log(tablog::ERROR, "Please provide a correct IPv4 address");
                }
            } else if (std::string(argv[index]).rfind("--file", 0) == 0) {
                std::string filepath = argv[index+1];
                if (std::filesystem::exists(filepath)) {
                    commandLineArguments.filePath = argv[index+1];
                } else {
                    logger->log(tablog::ERROR, "Please provide a correct Filepath");
                }
            }
        }

        if (commandLineArguments.tabloMasterIp.length() != 0) {
            Cli cli(&commandLineArguments);
        }
    } else {
        // Use config file (TODO)
        logger->log(tablog::ERROR, "failed!");
    }
    
    return 0;
}
