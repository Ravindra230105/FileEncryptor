#ifndef PROCESS_MANAGEMENT_HPP
#define PROCESS_MANAGEMENT_HPP

#include <pthread.h>
#include <semaphore.h>

#include <string>
#include <vector>

const int MAX_FILES = 4096;
const int MAX_PATH_LENGTH = 256;
const int MAX_WORKERS = 32;

class ProcessManagement {
public:
    ProcessManagement();
    ~ProcessManagement();

    bool isReady() const;
    void addTask(const std::string& path);
    void runWorkers(int workerCount, int key);
    int filesDoneBy(int worker) const;

private:
    // This struct lives in shared memory so parent and children all see the same queue.
    struct SharedData {
        char paths[MAX_FILES][MAX_PATH_LENGTH];
        int taskCount;
        int nextTask;
        int filesDone[MAX_WORKERS];
        pthread_mutex_t lock;
    };

    SharedData* shared;
    sem_t* remainingTasks;
    std::string semaphoreName;

    bool takeNextTask(std::string& path);
    void workerLoop(int worker, int key);
};

#endif
