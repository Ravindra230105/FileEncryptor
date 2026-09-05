#ifndef IO_HPP
#define IO_HPP

#include <string>
#include <vector>
using namespace std;

class IO {
public:
    static vector<string> listFiles(string dir);
    static bool readFile(string path, vector<char> &data);
    static bool writeFile(string path, vector<char> &data);
};

#endif
