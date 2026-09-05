#include "src/app/encryptDecrypt/Cryption.hpp"
#include "src/app/fileHandling/IO.hpp"
#include "src/app/processes/ProcessManagement.hpp"

#include <sys/time.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
using namespace std;

// gives the current time in milliseconds
double getTime() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec * 1000.0 + t.tv_usec / 1000.0;
}

// does all the files one by one in a single process
double runSequential(vector<string> files, int key) {
    double start = getTime();

    for (int i = 0; i < (int)files.size(); i++) {
        cryptFile(files[i], key);
    }

    double timeTaken = getTime() - start;
    cout << "Sequential (1 process)   : " << timeTaken << " ms" << endl;

    return timeTaken;
}

// puts all the files in the queue and lets the child processes do the work
double runParallel(vector<string> files, int key, int workers) {
    ProcessManagement pm;

    if (!pm.isReady()) {
        cout << "Shared memory could not be created" << endl;
        return 0;
    }

    for (int i = 0; i < (int)files.size(); i++) {
        pm.addFile(files[i]);
    }

    double start = getTime();
    pm.startWorkers(workers, key);
    double timeTaken = getTime() - start;

    cout << "Parallel (" << workers << " processes)  : " << timeTaken << " ms" << endl;

    for (int i = 0; i < workers; i++) {
        cout << "   worker " << i << " handled " << pm.getDoneCount(i) << " files" << endl;
    }

    return timeTaken;
}

int main(int argc, char *argv[]) {
    string dir = "";
    string mode = "parallel";
    int workers = 4;

    // folder name can be given directly or with --dir
    if (argc > 1 && argv[1][0] != '-') {
        dir = argv[1];
    }

    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "--dir") == 0) dir = argv[i + 1];
        if (strcmp(argv[i], "--mode") == 0) mode = argv[i + 1];
        if (strcmp(argv[i], "--workers") == 0) workers = atoi(argv[i + 1]);
    }

    if (workers < 1) workers = 1;
    if (workers > MAX_WORKERS) workers = MAX_WORKERS;

    if (dir == "") {
        cout << "Usage: ./encrypt_decrypt <folder> [--mode sequential|parallel|compare] [--workers N]" << endl;
        return 1;
    }

    vector<string> files = IO::listFiles(dir);
    if (files.size() == 0) {
        cout << "No files found in " << dir << endl;
        return 1;
    }

    int key = getKey();
    cout << "Found " << files.size() << " files in " << dir << endl;

    if (mode == "sequential") {
        runSequential(files, key);
        return 0;
    }

    if (mode == "parallel") {
        runParallel(files, key, workers);
        return 0;
    }

    // compare mode runs both of them. the second run also makes the files
    // normal again because XOR two times cancels out
    double t1 = runSequential(files, key);
    double t2 = runParallel(files, key, workers);

    if (t2 > 0) {
        cout << "Speedup: " << t1 / t2 << "x" << endl;
    }

    return 0;
}
