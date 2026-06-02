#include "cli.h"

#include <client_session_controller.h>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>

#include "network_manager.h"

Cli::Cli(struct Argv argv) {
  std::string tabloMaster = argv.tabloMaster;
  std::string filePath = argv.filePath;
  
  std::wcout << "Client! Tablo master at: " << tabloMaster.c_str() << std::endl;
  
  NetworkManager networkManager;

  if (networkManager.createSocket(tabloMaster) < 0) {
    std::wcout << "Create network manager failed!" << std::endl;
    return;
  }

  if (filePath.length() != 0) {
    std::wcout << "FilePath: " << filePath.c_str() << std::endl;
    std::ifstream file(filePath);
    
    if (file.is_open()) {
        std::string fileContent;
        std::string line;
        while (std::getline(file, line)) {
            fileContent = fileContent + line.c_str() + "\n";
        }
        file.close();
        
        Networking::Packet packet;
        Networking::Standard payload;
        payload.payload = fileContent;
        packet.payload = payload;
        
        networkManager.pushRequest(packet);
    }
  }
  
  while (true) {
    std::wcout << "Choose option\n(1) send Packet\n(2) read Packets\noption:";
    std::string option = "";
    std::cin >> option;
    if (option == "1") {
      std::string content;
      std::wcout << "Content: ";
      std::cin >> content;

      Networking::Packet packet;

      Networking::Standard payload;
      payload.payload = content;
      packet.payload = payload;

      networkManager.pushRequest(packet);
    } else if (option == "2") {
      if (networkManager.hasResponse()) {
        while (networkManager.hasResponse()) {
          ClientSessionController::Packet response = networkManager.popResponse();
          ClientSessionController::Standard responsePayload = std::get<ClientSessionController::Standard>(response.payload);

          std::wcout << "Response:\nID: " << response.id << "\nPayload: " << responsePayload.payload.c_str() << std::endl;
        }
      } else {
        std::wcout << "There are currently no packets to read!" << std::endl;
      }
    } else {
      std::wcout << "invalid" << std::endl;
    }
  }
}

