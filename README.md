# Parallel File Encryptor and Decryptor

A C++ command line tool that encrypts every file in a folder. It can do the work one file at a
time in a single process, or spread it across several child processes, so the two can be compared.

The point of the project is the operating system side of it: creating processes with `fork()`,
sharing a task queue between them through shared memory, and keeping that queue safe with a
semaphore and a mutex.

## How it works

The parent process builds a list of files and copies each path into a shared memory region created
with `mmap`. It then forks the worker processes. Because the memory is mapped with `MAP_SHARED`,
every child sees the same queue.

Each worker repeats the same loop:

1. `sem_trywait` on the task semaphore. If it fails, the queue is empty and the worker exits.
2. Lock the mutex, take the next file path, move the index forward, unlock.
3. Encrypt that file.

The parent then calls `waitpid` on each child so it knows when everything is finished.

Because workers take files whenever they are free rather than getting a fixed share up front, the
load balances itself. A run of 1000 files across 4 workers usually splits close to
`296 / 271 / 220 / 213` depending on how big each file is.

A normal mutex only works inside a single process, so it is created with the
`PTHREAD_PROCESS_SHARED` attribute, otherwise the children would each end up locking their own copy.

## The cipher

Every byte is XORed with a key read from a `.env` file (8717 is used if the file is missing).

XOR is its own inverse, so the same function encrypts and decrypts. Running the tool twice on the
same folder gives back the original files.

This is deliberately simple and is **not real encryption** - a single repeated byte is trivial to
break. The project is about process management, not cryptography.

## Building and running

```bash
make all

./encrypt_decrypt sample-data --mode compare --workers 4
```

| Flag | Meaning | Default |
|------|---------|---------|
| first argument | folder to process (or `--dir <path>`) | required |
| `--mode` | `sequential`, `parallel` or `compare` | `parallel` |
| `--workers` | number of child processes | 4 |

`compare` runs both versions and prints the speedup. The second run also restores the files,
since XORing twice with the same key cancels out.

Example output:

```
Found 200 files in test-data
Sequential (1 process)   : 132.153 ms
Parallel (8 processes)  : 51.5811 ms
   worker 0 handled 34 files
   worker 1 handled 29 files
   worker 2 handled 30 files
   worker 3 handled 26 files
   worker 4 handled 23 files
   worker 5 handled 22 files
   worker 6 handled 19 files
   worker 7 handled 17 files
Speedup: 2.56205x
```

That run was 200 files of 256 KB each on a 10 core Mac.

To make a folder to test with:

```bash
python3 makeDirs.py test-data 200 262144
```

## Notes and limits

- Speedup is well under the number of workers because reading and writing the files is serial work
  that cannot be split. With a cipher this cheap, most of the time goes on I/O rather than the CPU.
- Bigger files show the difference better than very small ones.
- Files are encrypted in place, so an interrupted run leaves them half done.
- Uses `fork`, `mmap` and POSIX semaphores, so it runs on Linux and macOS but not Windows.
- The whole file is read into memory, so a file has to fit in RAM.
