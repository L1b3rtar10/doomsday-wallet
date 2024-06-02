#include <iostream>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "seed_generator.h"
#include "../input/input_mgr.h"
#include "../crypto/hmac_sha512.h"
#include "../crypto/byteutils.h"

#define PBKDF2_ITERATIONS 10133 // Number of PBKDF2 iterations

#define INPUT_LEN 255   // Max length for each user input

optional<SeedGenerator> SeedGenerator::Make(char *filename, uint8_t* randomSeed)
{
    return SeedGenerator(filename, randomSeed);
}

void SeedGenerator::start(uint8_t* entropy)
{
    char buffer[BUF_LEN];
    uint8_t currentKey[KEY_SIZE] = {0x00};
    memcpy(currentKey, entropy, KEY_SIZE);

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
    
    while (fgets(buffer, BUF_LEN, fp)) {
        buffer[strcspn(buffer, "\n")] = 0;
        if (buffer[0] != '#' && strlen(buffer) > 0) {
            cout << "\n" << buffer << "\n";

            memset(input, '\0', INPUT_LEN);
            size_t len = 0;
            inputMgr->secureInput(input, len);

            uint8_t derivedKey[KEY_SIZE];
            if (!PKCS5_PBKDF2_HMAC(input, sizeof(input), currentKey, KEY_SIZE, PBKDF2_ITERATIONS, \
                                    EVP_sha512(), KEY_SIZE, derivedKey)) {
                std::cerr << "Error: PBKDF2 derivation failed\n";
                exit(EXIT_FAILURE);
            }

            memcpy(currentKey, derivedKey, KEY_SIZE);
        }
    }
    fclose(fp);
    memcpy(entropy, currentKey, KEY_SIZE);
    _seedInitialised = true;
    cout << "\n\nSeed generated, press return to continue...\n";
}

bool SeedGenerator::seedIsInitialised()
{
    return _seedInitialised;
}

void SeedGenerator::showPrompt()
{
    cout << "\033[2J\033[1;1H";
    cout << "* * * * * * * * * * * * * \n";
    cout << "* Initialise master key * \n";
    cout << "* * * * * * * * * * * * * \n";
}
