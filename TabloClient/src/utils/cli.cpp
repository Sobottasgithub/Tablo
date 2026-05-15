#include "cli.h"

#include <string>
#include <iostream>
#include <thread>

#include "networking.h"

Cli::Cli(std::string tabloMaster) {
  std::wcout << "Client! Tablo master at: " << tabloMaster.c_str() << std::endl;
  Networking networking;

  std::thread networkingThread(
      &Networking::networkingCycle,
      &networking,
      tabloMaster
  );
  
  while (true) {
    std::string content;
    std::wcout << "Content: ";
    std::cin >> content;
    std::wcout << std::endl;

    networking.pushOrder(1, content);

    std::wcout << "Waiting for solution..." << std::endl;
    while (!networking.hasSolution()) {} // Wait for solution

    std::wcout << networking.popSolution().c_str() << std::endl;
  }
}

