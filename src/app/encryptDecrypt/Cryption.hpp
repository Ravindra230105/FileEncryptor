#ifndef CRYPTION_HPP
#define CRYPTION_HPP

#include <string>

int readKey();
bool cryptFile(const std::string& path, int key);

#endif
