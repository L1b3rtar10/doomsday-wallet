#include <iostream>
#include <cassert>
#include "input/input_mgr.h"
#include "crypto/byteutils.h"
#include "features/seed_generator.h"
#include "features/descriptors.h"

using namespace std;

// 64 random bytes
#define RANDOM_SEED { \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  \
}

#define EXIT_CODE '5'

#define OPTIONS_MENU_NO_SEED "\n\n+++ Choose an option +++\n\
----- 1. Initialise master key\n"


void createMenu(bool seedInitialised, optional<DescriptorMgr> descriptorMgr) {
    char descriptor[MAX_DESCRIPTOR_LENGTH] = {'\0'};

    cout << "\033[2J\033[1;1H";
    cout << "* * * * * * * * * * * * * * *\n";
    cout << "*     Doomsday Wallet       *\n";
    cout << "* * * * * * * * * * * * * * *\n";
    cout << "\n";
    cout << "-- 1. Initialise master key" << (seedInitialised?" OK":"") << endl;

    if (descriptorMgr.has_value() && descriptorMgr->getAccountKey().has_value()) {
        char descriptorOutput[MAX_DESCRIPTOR_LENGTH] = {'\0'};
        descriptorMgr->getAccountKey()->getDescriptor(descriptorOutput);
        cout << "-- 2. Reference account keys: " << descriptorOutput << endl;
    } else {
        cout << "-- 2. Generate account keys" << endl;
    }
    cout << "-- 3. Export public key descriptors\n";
    cout << "-- 4. Export private key descriptor\n";
    cout << "-- 5. Exit\n";
    cout << "Please select an option: ";
}

void printMenu() {
    cout << "\033[2J\033[1;1H";
    cout << OPTIONS_MENU_NO_SEED << "----- " << EXIT_CODE << ". Exit\n\n+++ Selection: ";
    return;
}

constexpr bool isAllZero(const uint8_t array[], size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (array[i] != 0) {
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv) {
    static constexpr uint8_t randomSeed[] = RANDOM_SEED;
    static_assert(sizeof(randomSeed) == ENTROPY_SIZE, "RANDOM_SEED provided MUST be 64 bytes long");
    static_assert(!isAllZero(randomSeed, ENTROPY_SIZE), "RANDOM_SEED provided should be generated randomly");
    
    optional<SeedGenerator> seedGenerator = SeedGenerator::Make(argv[1]);
    optional<DescriptorMgr> descriptorMgr = nullopt;

    printMenu();
    
    char choice;
    scanf(" %c", &choice);

    while (choice != EXIT_CODE) {
        switch (choice) {
            case '1':
                if (seedGenerator.has_value()) {
                    seedGenerator->start(randomSeed);
                    descriptorMgr = DescriptorMgr(seedGenerator->getSeed(), ENTROPY_SIZE);
                    getchar();
                }
                break;
            case '2':
                if (descriptorMgr.has_value()) {
                    descriptorMgr->start();
                } else {
                    cout << "Master key must be initialised first.";
                }
                break;
            case '3':
                if (descriptorMgr.has_value()) {
                    descriptorMgr->exportDescriptors();
                } else {
                    cout << "Master key must be initialised first.";
                }
                break;
        }
        createMenu(seedGenerator->seedIsInitialised(), descriptorMgr);
        scanf(" %c", &choice);
    }

    return 0;
}
