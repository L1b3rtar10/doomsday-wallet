# https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/
# C++ is object-oriented C (has classes)
TARGET = doomsdaywallet
CC = g++

SUBMODULE_INCLUDE = $(CURDIR)/../secp256k1/include
export SUBMODULE_INCLUDE

LDFLAGS = -lsecp256k1 -lstdc++

CFLAGS = -g -Wall -Wextra -std=c++17 -lstdc++ -I$(SUBMODULE_INCLUDE)

# The list of your directories under the root directory:
SUBDIRS := utils crypto

SRCS := $(wildcard *.cpp $(addsuffix /*.cpp,$(SUBDIRS)))
OBJS := $(patsubst %.cpp,%.o,$(SRCS))

%.o: %.cpp
	@echo "\n\nHere is > $(SUBMODULE_INCLUDE)\n\n"
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	@echo "$(OBJS)";
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

.PHONY: clean
clean:
		@$(foreach dir,$(SUBDIRS),$(MAKE) -C$(dir) clean;)
		rm -rf build/
		rm -rf *.o
		rm -rf $(TARGET)
