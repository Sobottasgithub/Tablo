#include <iostream>
#include <tabcrypt.h>
#include "tabcrypt_test.h"


void handleLine(const std::string& line) {
    if (encrypt_mode) {
        std::cout << tabcrypt::encrypt(line, secret) << "\n";
    } else {
        try {
            std::cout << tabcrypt::decrypt(line, secret) << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Decryption failed: " << e.what() << "\n";
        }
    }
}


/*
 * Arguments:
 * tabcrypt_test <secret> [-d|-e]
 * Messages via stdin/stdout
 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <secret> [-d|-e]\n"
                "  -d: decrypt (default)\n"
                "  -e: encrypt\n";
        return EXIT_FAILURE;
    };
    std::string secret = argv[1];

    // parse argv[2]

    if (std::string(argv[2]) == "-e") {
        encrypt_mode = true;
    } else if (std::string(argv[2]) != "-d") {
        std::cerr << "Unknown option: " << argv[2] << "\n";
        return EXIT_FAILURE;
    }
}
