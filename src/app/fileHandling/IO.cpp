#include "IO.hpp"

#include <dirent.h>
#include <sys/stat.h>

#include <fstream>

// Walks the directory and returns the path of every regular file inside it.
std::vector<std::string> IO::listFiles(const std::string& directory) {
    std::vector<std::string> files;

    DIR* dir = opendir(directory.c_str());
    if (dir == NULL) {
        return files;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") {
            continue;
        }

        std::string fullPath = directory + "/" + name;

        struct stat info;
        if (stat(fullPath.c_str(), &info) == 0 && S_ISREG(info.st_mode)) {
            files.push_back(fullPath);
        }
    }

    closedir(dir);

    return files;
}

bool IO::readFile(const std::string& path, std::vector<char>& data) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    data.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    return true;
}

bool IO::writeFile(const std::string& path, const std::vector<char>& data) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    file.write(data.data(), data.size());

    return true;
}
