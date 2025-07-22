#ifndef libtabcrypt
#define libtabcrypt

#include <string>

std::string encrypt(const std::string &secret, const std::string &message);

std::string decrypt(const std::string &secret, const std::string &message);

#endif
