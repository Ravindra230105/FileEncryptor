CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall -pthread

SRC = main.cpp \
      src/app/fileHandling/IO.cpp \
      src/app/encryptDecrypt/Cryption.cpp \
      src/app/processes/ProcessManagement.cpp

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o encrypt_decrypt

clean:
	rm -f encrypt_decrypt

.PHONY: all clean
