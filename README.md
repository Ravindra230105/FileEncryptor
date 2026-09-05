# Parallel File Encryptor and Decryptor

A C++ command line tool that encrypts all the files present in a folder. It can do the work
in a single process one file at a time, or divide the work between several child processes,
so that both can be compared.

The main idea of the project is the operating system part: creating processes using `fork()`,
sharing a task queue between them using shared memory, and protecting that queue with a
semaphore and a mutex.

## How it works

The parent process makes a list of all the files and copies each path into a shared memory
region created by `mmap`. After that it creates the worker processes using `fork()`. Since the
memory is mapped with `MAP_SHARED`, every child sees the same queue.

Each worker repeats these steps:

1. Call `sem_trywait` on the semaphore. If it fails, the queue is empty so the worker exits.
2. Lock the mutex, take the next file path, move the index forward, unlock.
3. Encrypt that file.

At the end the parent calls `waitpid` for every child, so it knows when all the work is over.

The workers pick up a file whenever they are free instead of getting a fixed share at the
start, so the load gets balanced by itself. That is why the counts printed for each worker are
not equal.

One important point is the mutex attribute. A normal mutex works only inside a single process,
so it is created with `PTHREAD_PROCESS_SHARED`. Without that flag every child would lock its
own copy and the queue would not actually be protected.

## The cipher

Every byte of the file is XORed with a key which is read from a `.env` file (8717 is used if
the file is not there).

XOR is its own inverse, so the same function encrypts as well as decrypts. Running the tool
two times on the same folder gives back the original files.

This is kept simple on purpose and is **not real encryption** - a single repeating byte can be
broken very easily. The project is about process management, not cryptography.

## Building and running

```bash
make all

mkdir -p demo-data
for i in $(seq 1 800); do head -c 65536 /dev/urandom > demo-data/file$i.bin; done

./encrypt_decrypt demo-data --mode compare --workers 8
```

| Argument | Meaning | Default |
|----------|---------|---------|
| first argument | folder to process (or `--dir <path>`) | required |
| `--mode` | `sequential`, `parallel` or `compare` | `parallel` |
| `--workers` | number of child processes | 4 |

`compare` runs both versions and prints the speedup. It also brings the files back to normal,
because doing XOR two times with the same key cancels out.

Example output on a folder of 800 files of 64 KB each:

```
Found 800 files in demo-data
Sequential (1 process)   : 580.349 ms
Parallel (8 processes)  : 130.323 ms
   worker 0 handled 118 files
   worker 1 handled 104 files
   ...
Speedup: 4.45316x
```

## Notes and limits

- The speedup is less than the number of workers because reading and writing the files is work
  that cannot be divided. Adding more workers stops helping after a point.
- A folder with a few small files can even be slower in parallel than sequential, because
  creating the processes costs more than the work itself.
- Files are read and written one byte at a time. Reading a whole file in one call is faster,
  but then the program becomes limited by the disk instead of the CPU and the parallel version
  gains much less.
- Encryption happens in place, so if a run is stopped in the middle the files stay half done.
- The program does not remember whether a folder is already encrypted or not.
- Only the files directly inside the given folder are taken, sub folders are skipped.
- Uses `fork`, `mmap` and POSIX semaphores, so it works on Linux and macOS but not on Windows.
- The whole file is loaded into memory, so a file has to fit in RAM.
- Fixed limits: 4096 files, 256 character paths and 32 workers.
