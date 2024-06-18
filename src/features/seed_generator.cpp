#include <iostream>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "seed_generator.h"
#include "../input/input_mgr.h"
#include "../crypto/hmac_sha512.h"
#include "../crypto/byteutils.h"

#define PBKDF2_ITERATIONS 100000 // Number of PBKDF2 iterations

#define INPUT_LEN 64   // Max length for each user input

optional<SeedGenerator> SeedGenerator::Make(char *filename)
{
    return SeedGenerator(filename);
}

void SeedGenerator::start(const uint8_t* randomSeed)
{
    char buffer[BUF_LEN];
    memcpy(_masterSeed, randomSeed, ENTROPY_SIZE);

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

            if (!PKCS5_PBKDF2_HMAC(input, len, _masterSeed, ENTROPY_SIZE, PBKDF2_ITERATIONS, \
                                    EVP_sha512(), ENTROPY_SIZE, derivedKey)) {
                std::cerr << "Error: PBKDF2 derivation failed\n";
                exit(EXIT_FAILURE);
            }

            memcpy(_masterSeed, derivedKey, ENTROPY_SIZE);
        }
    }
    fclose(fp);
    _seedInitialised = true;
    cout << "\n\nSeed generated, press return to continue...\n";
}

bool SeedGenerator::seedIsInitialised()
{
    return _seedInitialised;
}

uint8_t* SeedGenerator::getSeed()
{
    return _masterSeed;
}

void SeedGenerator::showPrompt()
{
    cout << "\033[2J\033[1;1H";
    cout << "* * * * * * * * * * * * * \n";
    cout << "* Initialise master key * \n";
    cout << "* * * * * * * * * * * * * \n";
}
