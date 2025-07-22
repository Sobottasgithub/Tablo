#ifndef libtabcrypt
#define libtabcrypt

#include <string>

namespace tabcrypt {
    std::string encrypt(const std::string &secret, const std::string &message);

    std::string decrypt(const std::string &secret, const std::string &message);
}

#endif
