#include <iostream>
#include <cassert>
#include "input/input_mgr.h"
#include "crypto/byteutils.h"
#include "features/seed_generator.h"
#include "features/descriptors.h"

using namespace std;

// 64 random bytes
#define RANDOM_SEED {  0xa9, 0x18, 0x6a, 0x26, 0x5e, 0xc9, 0xe8, 0x3c, 0x8e, 0x8a, 0x70, 0xc3,\
0x9a, 0xf0, 0xb6, 0xc0, 0x54, 0x27, 0x9b, 0x6e, 0x5b, 0x39, 0x49, 0x85, 0x64, 0x4d, 0xfe, 0x6e, 0x5e,\
0xdc, 0x37, 0x0a, 0x24, 0x20, 0xf0, 0xa9, 0x1e, 0xeb, 0x09, 0x43, 0x59, 0x30, 0xa4, 0xcd, 0x65, 0x18,\
0xd2, 0x0c, 0x71, 0xfa, 0x87, 0x29, 0x96, 0x1d, 0x73, 0x2a, 0xc9, 0x43, 0x5b, 0xce, 0xed, 0x00, 0x1b, 0x25 }

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

int main(int argc, char **argv) {
    uint8_t randomSeed[] = RANDOM_SEED;
    static_assert(sizeof(randomSeed) == ENTROPY_SIZE, "RANDOM_SEED provided MUST be 64 bytes long");
    
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
        }
        createMenu(seedGenerator->seedIsInitialised(), descriptorMgr);
        scanf(" %c", &choice);
    }

    return 0;
}
