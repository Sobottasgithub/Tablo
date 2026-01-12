#include "cli.h"

#include <ostream>
#include <string>
#include <iostream>
#include <regex>
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
    int method;
    std::wcout << "Method: ";
    std::cin >> method;
    std::wcout << std::endl;

    std::string content;
    std::wcout << "Content: ";
    std::cin >> content;
    std::wcout << std::endl;

    networking.pushOrder(method, content);

    std::wcout << "Waiting for solution..." << std::endl;
    while (!networking.hasSolution()) {} // Wait for solution

    std::wcout << networking.popSolution().c_str() << std::endl;
  }
}

