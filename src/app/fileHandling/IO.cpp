#include "IO.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>

vector<string> IO::listFiles(string dir) {
    vector<string> files;

    DIR *d = opendir(dir.c_str());
    if (d == NULL) {
        return files;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        string name = entry->d_name;

        if (name == "." || name == "..") {
            continue;
        }

        string path = dir + "/" + name;

        struct stat st;
        if (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            files.push_back(path);
        }
    }

    closedir(d);
    return files;
}

bool IO::readFile(string path, vector<char> &data) {
    ifstream fin(path.c_str(), ios::binary);
    if (!fin.is_open()) {
        return false;
    }

    char ch;
    while (fin.get(ch)) {
        data.push_back(ch);
    }

    fin.close();
    return true;
}

bool IO::writeFile(string path, vector<char> &data) {
    ofstream fout(path.c_str(), ios::binary | ios::trunc);
    if (!fout.is_open()) {
        return false;
    }

    for (int i = 0; i < (int)data.size(); i++) {
        fout.put(data[i]);
    }

    fout.close();
    return true;
}
