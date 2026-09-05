#include "ProcessManagement.hpp"
#include "../encryptDecrypt/Cryption.hpp"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>

ProcessManagement::ProcessManagement() {
    // MAP_SHARED means the memory stays visible to every process we fork later.
    shared = (SharedData*) mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (shared == MAP_FAILED) {
        shared = NULL;
        remainingTasks = SEM_FAILED;
        return;
    } 

    memset(shared, 0, sizeof(SharedData));

    // A normal mutex only works inside one process, so it has to be marked as shared.
    pthread_mutexattr_t attributes;
    pthread_mutexattr_init(&attributes);
    pthread_mutexattr_setpshared(&attributes, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shared->lock, &attributes);
    pthread_mutexattr_destroy(&attributes);

    semaphoreName = "/encryptor-" + std::to_string(getpid());
    sem_unlink(semaphoreName.c_str());
    remainingTasks = sem_open(semaphoreName.c_str(), O_CREAT, 0644, 0);
}

ProcessManagement::~ProcessManagement() {
    if (remainingTasks != SEM_FAILED) {
        sem_close(remainingTasks);
        sem_unlink(semaphoreName.c_str());
    }

    if (shared != NULL) {
        munmap(shared, sizeof(SharedData));
    }
}

bool ProcessManagement::isReady() const {
    return shared != NULL && remainingTasks != SEM_FAILED;
}

void ProcessManagement::addTask(const std::string& path) {
    if (shared->taskCount >= MAX_FILES) {
        return;
    }

    strncpy(shared->paths[shared->taskCount], path.c_str(), MAX_PATH_LENGTH - 1);
    shared->taskCount++;

    sem_post(remainingTasks);
}

// Claims one file from the queue. Returns false once the queue is empty.
bool ProcessManagement::takeNextTask(std::string& path) {
    if (sem_trywait(remainingTasks) != 0) {
        return false;
    }

    pthread_mutex_lock(&shared->lock);
    path = shared->paths[shared->nextTask];
    shared->nextTask++;
    pthread_mutex_unlock(&shared->lock);

    return true;
}

void ProcessManagement::workerLoop(int worker, int key) {
    std::string path;

    while (takeNextTask(path)) {
        if (cryptFile(path, key)) {
            shared->filesDone[worker]++;
        }
    }
}

void ProcessManagement::runWorkers(int workerCount, int key) {
    std::vector<pid_t> children;

    for (int worker = 0; worker < workerCount; worker++) {
        pid_t pid = fork();

        if (pid == 0) {
            workerLoop(worker, key);
            _exit(0);
        }

        if (pid > 0) {
            children.push_back(pid);
        }
    }

    for (size_t i = 0; i < children.size(); i++) {
        waitpid(children[i], NULL, 0);
    }
}

int ProcessManagement::filesDoneBy(int worker) const {
    return shared->filesDone[worker];
}
