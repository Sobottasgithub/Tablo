#include "cli.h"

#include <client_session_controller.h>
#include <string>
#include <iostream>
#include <thread>

#include "network_manager.h"

Cli::Cli(std::string tabloMaster) {
  std::wcout << "Client! Tablo master at: " << tabloMaster.c_str() << std::endl;
  NetworkManager networkManager;

  std::thread networkingThread(
      &NetworkManager::networkingCycle,
      &networkManager,
      tabloMaster
  );
  
  while (true) {
    std::string content;
    std::wcout << "Content: ";
    std::cin >> content;
    std::wcout << std::endl;

    // networkManager.pushRequest();

    std::wcout << "Waiting for solution..." << std::endl;
    while (!networkManager.hasResponse()) {} // Wait for solution

    //TODO: pop response and display
    
  }
}

