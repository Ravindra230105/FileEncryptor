import os
import random
import string
import sys

# Creates a folder of random text files to test the encryptor with.
# Usage: python3 makeDirs.py [folder] [file count] [size in bytes]
def makeFiles(path, count, size):
    os.makedirs(path, exist_ok=True)

    for i in range(count):
        filename = os.path.join(path, f"test{i + 1}.txt")
        with open(filename, "w") as file:
            file.write(''.join(random.choices(string.ascii_uppercase + string.digits, k=size)))

    print(f"Created {count} files of {size} bytes in {path}")

if __name__ == "__main__":
    folder = sys.argv[1] if len(sys.argv) > 1 else "sample-data"
    count = int(sys.argv[2]) if len(sys.argv) > 2 else 1000
    size = int(sys.argv[3]) if len(sys.argv) > 3 else 1000

    makeFiles(folder, count, size)
