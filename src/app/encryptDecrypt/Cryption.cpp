#include "Cryption.hpp"
#include "../fileHandling/IO.hpp"
#include <fstream>
#include <vector>

// key is read from the .env file, if not found then default one is used
int getKey() {
    int key = 8717;

    ifstream fin(".env");
    if (fin.is_open()) {
        fin >> key;
        fin.close();
    }

    return key % 256;
}

// same function is used for encrypt and decrypt
// because doing XOR two times with the same key gives back the original data
bool cryptFile(string path, int key) {
    vector<char> data;

    if (!IO::readFile(path, data)) {
        return false;
    }

    for (int i = 0; i < (int)data.size(); i++) {
        data[i] = data[i] ^ key;
    }

    return IO::writeFile(path, data);
}
