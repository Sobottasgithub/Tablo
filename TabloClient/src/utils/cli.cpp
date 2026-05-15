#include "cli.h"

#include <client_session_controller.h>
#include <string>
#include <iostream>
#include <thread>

#include "network_manager.h"

Cli::Cli(std::string tabloMaster) {
  std::wcout << "Client! Tablo master at: " << tabloMaster.c_str() << std::endl;
  NetworkManager networkManager;

  if (networkManager.createSocket(tabloMaster) < 0) {
    return;
  }
  
  while (true) {
    std::string content;
    std::wcout << "Content: ";
    std::cin >> content;
    std::wcout << std::endl;

    ClientSessionController::Packet packet;
    packet.id = 1; //TODO: autoincrement in TTP2

    ClientSessionController::Standard payload;
    payload.payload = content;
    packet.payload = payload;

    networkManager.pushRequest(packet);

    std::wcout << "Waiting for solution..." << std::endl;
    while (!networkManager.hasResponse()) {} // Wait for solution

    ClientSessionController::Packet response = networkManager.popResponse();
    ClientSessionController::Standard responsePayload = std::get<ClientSessionController::Standard>(response.payload);

    std::wcout << "response: " << responsePayload.payload.c_str() << std::endl;    
  }
}

