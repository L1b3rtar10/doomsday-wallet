TARGET = doomsdaywallet
CC = g++

SUBMODULE_INCLUDE = $(CURDIR)/../secp256k1/include
export SUBMODULE_INCLUDE

LDFLAGS = -lsecp256k1 -lstdc++ -lcrypto -lm -lcurl -L${SSL_PATH}/lib -L$(CURDIR)/../secp256k1/lib

# Common flags
CFLAGS = -Wall -Wextra -std=c++17 -I$(SUBMODULE_INCLUDE) -I${SSL_PATH}/include

# Debug flags
DEBUG_CFLAGS =

# The list of your directories under the root directory:
SUBDIRS := utils crypto input features bitcoin

# Ensure the build directory exists
$(shell mkdir -p ../build $(addprefix ../build/,$(SUBDIRS)))

# Recursively find all .cpp files
SRCS := $(wildcard *.cpp $(addsuffix /*.cpp,$(SUBDIRS)))

# Transform source file paths to object file paths in the build directory
OBJS := $(patsubst %.cpp,../build/%.o,$(SRCS))

# Ensure corresponding build subdirectories exist
$(shell mkdir -p $(addprefix ../build/,$(SUBDIRS)))

# Target for object files
../build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $< $(CFLAGS) $(DEBUG_CFLAGS)

# Targets for normal and debug builds
all: $(TARGET)

debug: DEBUG_CFLAGS += -g
debug: clean $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf ../build/
	rm -rf $(TARGET)

deploy:
	make clean
	tar -czvf doomsdayDeploy.tar.gz .
	scp -P1985 doomsdayDeploy.tar.gz iceman@192.168.1.200:~
