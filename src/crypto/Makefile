# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/
# C++ is object-oriented C (has classes)
CC = g++
CFLAGS = -g -Wall -Wextra -std=c++17 -lstdc++ -I$(SUBMODULE_INCLUDE)

DEPS = byteutils.h sha-256.h base58.h hmac_sha512.h ripemd160.h sha512.h key.h bech32.h
OBJ = byteutils.o sha-256.o base58.o hmac_sha512.o ripemd160.o sha512.o key.o bech32.o

%.o: %.cpp $(DEPS)
	@echo "\n\nCRYPTO Here is > $(SUBMODULE_INCLUDE)\n\n"
	$(CC) -c -o $@ $< $(CFLAGS)

crypto: $(OBJ)
	$(CC) -c -o $@ $^ $(LIBS)

.PHONY: clean
clean:
	rm -rf *.o
