#include <iostream>
#include <string>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "seed_generator.h"
#include "seed.h"
#include "../input/input_mgr.h"
#include "../crypto/hmac_sha512.h"
#include "../crypto/byteutils.h"

#define PBKDF2_ITERATIONS 100000 // Number of PBKDF2 iterations

#define BUF_LEN 256
#define INPUT_LEN 64   // Max length for each user input

SeedGenerator SeedGenerator::Make(char *filename)
{
    return SeedGenerator(filename);
}

bool SeedGenerator::randomSeedIsInitialised() {
    uint8_t randomSeed[] = RANDOM_SEED;
    if (sizeof(randomSeed) != ENTROPY_SIZE) {
        cout << endl;
        cout << "RANDOM_SEED provided MUST be 64 bytes long, but is " << sizeof(randomSeed) << " instead.";
        return false;
    }
    if (isAllZero(randomSeed, ENTROPY_SIZE)) {
        cout << endl;
        cout << "RANDOM_SEED should be generated randomly";
        return false;
    }
    memset(randomSeed, 0x00, ENTROPY_SIZE);
    return true;
}

void SeedGenerator::start(uint8_t* masterSeed, uint8_t* lightningMasterSeed)
{
    uint8_t randomSeed[] = RANDOM_SEED;
    char buffer[BUF_LEN];
    memcpy(masterSeed, randomSeed, ENTROPY_SIZE);

    FILE* fp;
    fp = fopen(_filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "can't open file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    showPrompt();
    getchar();
    
    optional<InputMgr> inputMgr = InputMgr::Make(0);

    char input[INPUT_LEN] = {'\0'};
    size_t len = 0;

    uint8_t derivedKey[ENTROPY_SIZE];
    
    while (fgets(buffer, BUF_LEN, fp)) {
        buffer[strcspn(buffer, "\n")] = 0;
        if (buffer[0] != '#' && strlen(buffer) > 0) {
            cout << "\n" << buffer << "\n";

            memset(input, '\0', INPUT_LEN);
            inputMgr->secureInput(input, len);

            if (!PKCS5_PBKDF2_HMAC(input, len, masterSeed, ENTROPY_SIZE, PBKDF2_ITERATIONS, \
                                    EVP_sha512(), ENTROPY_SIZE, derivedKey)) {
                std::cerr << "Error: PBKDF2 derivation failed\n";
                exit(EXIT_FAILURE);
            }

            memcpy(masterSeed, derivedKey, ENTROPY_SIZE);
        }
    }
    CHMAC_SHA512{masterSeed, ENTROPY_SIZE}.Write(randomSeed, ENTROPY_SIZE).Finalize(lightningMasterSeed);
    fclose(fp);
    _seedInitialised = true;
}

bool SeedGenerator::seedIsInitialised()
{
    return _seedInitialised;
}

uint8_t* SeedGenerator::getSeed()
{
    return _masterSeed;
}

bool SeedGenerator::isAllZero(const uint8_t array[], size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (array[i] != 0) {
            return false;
        }
    }
    return true;
}


void SeedGenerator::showPrompt()
{
    cout << "\033[2J\033[1;1H";
    cout << "* * * * * * * * * * * * * \n";
    cout << "* Initialise master key * \n";
    cout << "* * * * * * * * * * * * * \n";
}
