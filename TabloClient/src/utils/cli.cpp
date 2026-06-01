#include "cli.h"

#include <client_session_controller.h>
#include <string>
#include <iostream>
#include <thread>

#include "network_manager.h"

Cli::Cli(struct Argv argv) {
  std::string tabloMaster = argv.tabloMaster;
  std::string filePath = argv.filePath;
  
  std::wcout << "Client! Tablo master at: " << tabloMaster.c_str() << std::endl;

  if (filePath.length() != 0) {
    std::wcout << "FilePath: " << filePath.c_str() << std::endl;
  }
  
  NetworkManager networkManager;

  if (networkManager.createSocket(tabloMaster) < 0) {
    std::wcout << "Create network manager failed!" << std::endl;
    return;
  }
  
  while (true) {
    std::string content;
    std::wcout << "Content: ";
    std::cin >> content;

    Networking::Packet packet;

    Networking::Standard payload;
    payload.payload = content;
    packet.payload = payload;

    networkManager.pushRequest(packet);

    std::wcout << "Waiting for solution..." << std::endl;
    while (!networkManager.hasResponse()) {} // Wait for solution

    ClientSessionController::Packet response = networkManager.popResponse();
    ClientSessionController::Standard responsePayload = std::get<ClientSessionController::Standard>(response.payload);

    std::wcout << "Response:\nID: " << response.id << "\nPayload: " << responsePayload.payload.c_str() << std::endl;    
  }
}

