CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall -pthread

TARGET = encrypt_decrypt

SRC = main.cpp \
      src/app/fileHandling/IO.cpp \
      src/app/encryptDecrypt/Cryption.cpp \
      src/app/processes/ProcessManagement.cpp

OBJ = $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
