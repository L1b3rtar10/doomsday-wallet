# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/
# C++ is object-oriented C (has classes)
TARGET = test
CC = gcc
CFLAGS = -g -Wall -Wextra -std=c++17 -lstdc++ -lm -lsecp256k1 -lssl -lcrypto -I/usr/local/lib

$(TARGET):
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp
	./$(TARGET)

.PHONY: clean
clean:
	rm -rf test test.dSYM
