#include <iostream>
#include <cassert>
#include "input/input_mgr.h"

#include "features/seed_generator.h"

using namespace std;

// 64 random bytes
#define RANDOM_SEED {  0xa9, 0x18, 0x6a, 0x26, 0x5e, 0xc9, 0xe8, 0x3c, 0x8e, 0x8a, 0x70, 0xc3,\
0x9a, 0xf0, 0xb6, 0xc0, 0x54, 0x27, 0x9b, 0x6e, 0x5b, 0x39, 0x49, 0x85, 0x64, 0x4d, 0xfe, 0x6e, 0x5e,\
0xdc, 0x37, 0x0a, 0x24, 0x20, 0xf0, 0xa9, 0x1e, 0xeb, 0x09, 0x43, 0x59, 0x30, 0xa4, 0xcd, 0x65, 0x18,\
0xd2, 0x0c, 0x71, 0xfa, 0x87, 0x29, 0x96, 0x1d, 0x73, 0x2a, 0xc9, 0x43, 0x5b, 0xce, 0xed, 0x00, 0x1b, 0x25 }

#define EXIT_CODE '9'

#define OPTIONS_MENU_NO_SEED "\n\n+++ Choose an option +++\n\
----- 1. Initialise master key\n"

#define OPTIONS_MENU "\n\n+++ Choose an option +++\n\
----- 1. Initialise master key - OK\n\
----- 2. Create safe wallet\n\
----- 3. Send amount to address\n\
----- 4. Decode transaction\n\
----- 5. Select default tx fee\n\
----- 6. Generate new address\n"

void printMenu(bool seedInitialised) {
    cout << "\033[2J\033[1;1H";
    cout << (seedInitialised?OPTIONS_MENU:OPTIONS_MENU_NO_SEED) << "----- " << EXIT_CODE << ". Exit\n\n+++ Selection: ";
    return;
}

int main(int argc, char **argv) {
    uint8_t randomSeed[] = RANDOM_SEED;
    static_assert(sizeof(randomSeed) == KEY_SIZE, "Random seed provided should be 64 bytes long");
    uint8_t masterSeed[KEY_SIZE] = {'\0'};
    
    optional<SeedGenerator> seedGenerator = SeedGenerator::Make(argv[1], randomSeed);

    cout << "* * * * * * * * * * * * * * * *\n";
    cout << "*  Welcome to Doomsday Wallet *\n";
    cout << "* * * * * * * * * * * * * * * *\n";

    printMenu(false);
    char choice;

    while ((choice = getchar()) != EXIT_CODE) {
        switch (choice) {
            case '1': {
                if (seedGenerator.has_value()) {
                    seedGenerator->start(masterSeed);
                }
                break;
            }
        }
        printMenu(seedGenerator->seedIsInitialised());
    }

    return 0;
}
