#include "ProcessManagement.hpp"
#include "../encryptDecrypt/Cryption.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <vector>

ProcessManagement::ProcessManagement() {
    // MAP_SHARED is important, otherwise every child will get its own
    // copy of this memory and they will not see each other's changes
    data = (SharedData *) mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (data == MAP_FAILED) {
        data = NULL;
        sem = SEM_FAILED;
        return;
    }

    memset(data, 0, sizeof(SharedData));

    // a normal mutex works only inside one process, so we have to
    // set this attribute to make it work between processes also
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&data->lock, &attr);
    pthread_mutexattr_destroy(&attr);

    // pid is added in the name so that two runs do not clash
    semName = "/encryptor-" + to_string(getpid());

    sem_unlink(semName.c_str());
    sem = sem_open(semName.c_str(), O_CREAT, 0644, 0);
}

ProcessManagement::~ProcessManagement() {
    if (sem != SEM_FAILED) {
        sem_close(sem);
        sem_unlink(semName.c_str());
    }

    if (data != NULL) {
        munmap(data, sizeof(SharedData));
    }
}

bool ProcessManagement::isReady() {
    return data != NULL && sem != SEM_FAILED;
}

// parent adds all the files in the queue before creating the workers
void ProcessManagement::addFile(string path) {
    if (data->total >= MAX_FILES) {
        return;
    }

    strncpy(data->files[data->total], path.c_str(), MAX_PATH_LEN - 1);
    data->total++;

    sem_post(sem);
}

// takes one file from the queue, returns false if queue is empty
bool ProcessManagement::getNextFile(string &path) {
    // semaphore tells us whether any file is left or not
    if (sem_trywait(sem) != 0) {
        return false;
    }

    // mutex tells us which file we get, without this two workers
    // can read the same index and encrypt the same file twice
    pthread_mutex_lock(&data->lock);
    path = data->files[data->next];
    data->next++;
    pthread_mutex_unlock(&data->lock);

    return true;
}

void ProcessManagement::doWork(int id, int key) {
    string path;

    while (getNextFile(path)) {
        if (cryptFile(path, key)) {
            data->done[id]++;
        }
    }
}

void ProcessManagement::startWorkers(int workers, int key) {
    vector<int> pids;

    for (int i = 0; i < workers; i++) {
        int pid = fork();

        // fork returns 0 in the child and child's pid in the parent
        if (pid == 0) {
            doWork(i, key);
            _exit(0);
        }

        if (pid > 0) {
            pids.push_back(pid);
        }
    }

    // parent waits here till all the children finish their work
    for (int i = 0; i < (int)pids.size(); i++) {
        waitpid(pids[i], NULL, 0);
    }
}

int ProcessManagement::getDoneCount(int id) {
    return data->done[id];
}
