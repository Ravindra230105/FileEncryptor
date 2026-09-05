#ifndef CRYPTION_HPP
#define CRYPTION_HPP

#include <string>
using namespace std;

int getKey();
bool cryptFile(string path, int key);

#endif
