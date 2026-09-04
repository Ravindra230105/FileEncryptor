#ifndef IO_HPP
#define IO_HPP

#include <string>
#include <vector>

class IO {
public:
    static std::vector<std::string> listFiles(const std::string& directory);
    static bool readFile(const std::string& path, std::vector<char>& data);
    static bool writeFile(const std::string& path, const std::vector<char>& data);
};

#endif
