#include "cli.h"

#include <client_session_controller.h>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <variant>

#include "network_manager.h"

Cli::Cli(Argv* argv) {
  std::string tabloMasterIp = argv->tabloMasterIp;
  std::string filePath = argv->filePath;
  
  std::wcout << "Client! Tablo master at: " << tabloMasterIp.c_str() << std::endl;
  
  NetworkManager networkManager;

  if (networkManager.createSocket(tabloMasterIp) < 0) {
    std::wcout << "Create network manager failed!" << std::endl;
    return;
  }

  sendFile(filePath, &networkManager);
  
  while (true) {
    std::wcout << "Choose option\n(1) send Packet\n(2) read Packets\n(3) send File\n(4) get Viewport\noption:";
    std::string option = "";
    std::cin >> option;
    if (option == "1") {
      std::string content;
      std::wcout << "Content: ";
      std::cin >> content;

      ttp2::Networking::Packet packet;

      ttp2::Networking::Standard payload;
      payload.payload = content;
      packet.payload = payload;

      networkManager.pushRequest(packet);
    } else if (option == "2") {
      if (networkManager.hasResponse()) {
        while (networkManager.hasResponse()) {
          ttp2::ClientSessionController::Packet response = networkManager.popResponse();
          
          if (std::holds_alternative<ttp2::ClientSessionController::Standard>(response.payload)) {
            ttp2::ClientSessionController::Standard responsePayload = std::get<ttp2::ClientSessionController::Standard>(response.payload);
            std::wcout << "Response:\nID: " << response.id << "\nPayload: " << responsePayload.payload.c_str() << std::endl;
          } else if (std::holds_alternative<ttp2::ClientSessionController::File>(response.payload)) {
            ttp2::ClientSessionController::File responsePayload = std::get<ttp2::ClientSessionController::File>(response.payload);
            std::wcout << "Response:\nID: " << response.id
                       << "\n----payload----\nFilePath: " << responsePayload.filePath.c_str()
                       << "\nStart: " << responsePayload.start
                       << "\nEnd: " << responsePayload.end
                       << "\nPayload: " << responsePayload.payload.c_str()
                       << "\n---------------" << std::endl; 
          } else if (std::holds_alternative<ttp2::ClientSessionController::Viewport>(response.payload)) {
            ttp2::ClientSessionController::Viewport responseViewport = std::get<ttp2::ClientSessionController::Viewport>(response.payload);
            if (responseViewport.payload.length() > 0) {
              std::wcout << responseViewport.payload.c_str() << std::endl;
            } else {
              std::wcout << "Empty Viewport" << std::endl;
            }
          }
        }
      } else {
        std::wcout << "There are currently no packets to read!" << std::endl;
      }
    } else if (option == "3") {
      std::string filePath;
      std::wcout << "CSV Filepath:";
      std::cin >> filePath;
      
      sendFile(filePath, &networkManager);
    } else if (option == "4") {
      int xStart;
      int xEnd;
      int yStart;
      int yEnd;

      std::wcout << "xStart:";
      std::cin >> xStart;
      std::wcout << "xEnd:";
      std::cin >> xEnd;
      std::wcout << "yStart:";
      std::cin >> yStart;
      std::wcout << "yEnd:";
      std::cin >> yEnd;
      
      ttp2::Networking::Packet packet;
      ttp2::Networking::Viewport payload;
      payload.xStart = xStart;
      payload.xEnd = xEnd;
      payload.yStart = yStart;
      payload.yEnd = yEnd;
      packet.payload = payload;
    
      networkManager.pushRequest(packet);
      std::wcout << "Send Viewport request!" << std::endl;
    } else {
      std::wcout << "invalid" << std::endl;
    }
  }
}

void Cli::sendFile(std::string filePath, NetworkManager* networkManager) {
  if (filePath.length() != 0) {
    std::ifstream file(filePath);
    
    if (file.is_open()) {
        std::string fileContent;
        std::string line;
        int lineCount = 0;
        while (std::getline(file, line)) {
          lineCount++;
          fileContent = fileContent + line.c_str() + "\n";
        }
        file.close();
        
        ttp2::Networking::Packet packet;
        ttp2::Networking::File payload;
        payload.filePath = filePath;
        payload.start = 0;
        payload.end = lineCount;
        payload.payload = fileContent;
        packet.payload = payload;
        
        networkManager->pushRequest(packet);
        std::wcout << "Send file with FilePath: " << filePath.c_str() << std::endl;
    }
  }
}
