#include <iostream>
#include <cstring>

#include "networking.h"

using namespace std;

Networking::Networking() {
  std::wcout << "Start tablo client socket..." << endl;
}

Networking::Networking(std::string tabloMaster) {
  std::wcout << "tablo Master: " << tabloMaster.c_str() << endl;
}
