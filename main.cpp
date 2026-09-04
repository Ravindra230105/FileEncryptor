#include "src/app/encryptDecrypt/Cryption.hpp"
#include "src/app/fileHandling/IO.hpp"
#include "src/app/processes/ProcessManagement.hpp"

#include <sys/time.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

double currentMillis() {
    struct timeval now;
    gettimeofday(&now, NULL);

    return now.tv_sec * 1000.0 + now.tv_usec / 1000.0;
}

// Runs every file one after another in this single process.
double runSequential(const std::vector<std::string>& files, int key) {
    double start = currentMillis();

    for (size_t i = 0; i < files.size(); i++) {
        cryptFile(files[i], key);
    }

    double elapsed = currentMillis() - start;
    std::cout << "Sequential (1 process)   : " << elapsed << " ms" << std::endl;

    return elapsed;
}

// Puts every file on the shared queue and lets a pool of child processes drain it.
double runParallel(const std::vector<std::string>& files, int key, int workerCount) {
    ProcessManagement manager;
    if (!manager.isReady()) {
        std::cout << "Could not set up shared memory" << std::endl;
        return 0;
    }

    for (size_t i = 0; i < files.size(); i++) {
        manager.addTask(files[i]);
    }

    double start = currentMillis();
    manager.runWorkers(workerCount, key);
    double elapsed = currentMillis() - start;

    std::cout << "Parallel (" << workerCount << " processes)  : " << elapsed << " ms" << std::endl;

    for (int worker = 0; worker < workerCount; worker++) {
        std::cout << "   worker " << worker << " handled "
                  << manager.filesDoneBy(worker) << " files" << std::endl;
    }

    return elapsed;
}

int main(int argc, char* argv[]) {
    std::string directory;
    std::string mode = "parallel";
    int workerCount = 4;

    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "--dir") == 0) directory = argv[i + 1];
        if (strcmp(argv[i], "--mode") == 0) mode = argv[i + 1];
        if (strcmp(argv[i], "--workers") == 0) workerCount = atoi(argv[i + 1]);
    }

    if (workerCount < 1) {
        workerCount = 1;
    }

    if (workerCount > MAX_WORKERS) {
        workerCount = MAX_WORKERS;
    }

    if (directory.empty()) {
        std::cout << "Usage: ./encrypt_decrypt --dir <path> [--mode sequential|parallel|compare]"
                  << " [--workers N]" << std::endl;
        return 1;
    }

    std::vector<std::string> files = IO::listFiles(directory);
    if (files.empty()) {
        std::cout << "No files found in " << directory << std::endl;
        return 1;
    }

    int key = readKey();
    std::cout << "Found " << files.size() << " files in " << directory << std::endl;

    if (mode == "sequential") {
        runSequential(files, key);
        return 0;
    }

    if (mode == "parallel") {
        runParallel(files, key, workerCount);
        return 0;
    }

    // Compare mode runs both. The second run also puts the files back to normal,
    // because XOR with the same key undoes the first run.
    double sequentialTime = runSequential(files, key);
    double parallelTime = runParallel(files, key, workerCount);

    if (parallelTime > 0) {
        std::cout << "Speedup: " << sequentialTime / parallelTime << "x" << std::endl;
    }

    return 0;
}
