#ifndef PROCESS_MANAGEMENT_HPP
#define PROCESS_MANAGEMENT_HPP

#include <pthread.h>
#include <semaphore.h>
#include <string>
using namespace std;

#define MAX_FILES 4096
#define MAX_PATH_LEN 256
#define MAX_WORKERS 32

// this structure is kept in shared memory so that all the
// child processes can see the same queue
struct SharedData {
    char files[MAX_FILES][MAX_PATH_LEN];
    int total;
    int next;
    int done[MAX_WORKERS];
    pthread_mutex_t lock;
};

class ProcessManagement {
public:
    ProcessManagement();
    ~ProcessManagement();

    bool isReady();
    void addFile(string path);
    void startWorkers(int workers, int key);
    int getDoneCount(int id);

private:
    SharedData *data;
    sem_t *sem;
    string semName;

    bool getNextFile(string &path);
    void doWork(int id, int key);
};

#endif
