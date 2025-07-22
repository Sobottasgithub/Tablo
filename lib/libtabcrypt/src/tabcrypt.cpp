#include <iostream>
#include "tabcrypt.h"

int main() {
  std::cout << "Hello, World!" << std::endl;
}

namespace tabcrypt {
  std::string encrypt(const std::string &secret, const std::string &message) {
    return message;
  }

  std::string decrypt(const std::string &secret, const std::string &message) {
    return message;
  }
}
