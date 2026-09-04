#include "Cryption.hpp"
#include "../fileHandling/IO.hpp"

#include <fstream>
#include <vector>

// Reads the key from the .env file, falling back to a default if it is missing.
int readKey() {
    std::ifstream envFile(".env");
    int key = 8717;

    if (envFile.is_open()) {
        envFile >> key;
    }

    return key % 256;
}

// XOR is its own inverse, so this one function both encrypts and decrypts.
bool cryptFile(const std::string& path, int key) {
    std::vector<char> data;

    if (!IO::readFile(path, data)) {
        return false;
    }

    for (size_t i = 0; i < data.size(); i++) {
        data[i] = data[i] ^ key;
    }

    return IO::writeFile(path, data);
}
