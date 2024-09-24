#ifndef SEED_GENERATOR_H
#define SEED_GENERATOR_H

#define BUF_LEN 256

#include <optional>
#include "../crypto/key.h"

using namespace std;


class SeedGenerator
{    
    private:

        char* _filename;
        uint8_t _masterSeed[ENTROPY_SIZE] = {'\0'};
        bool _seedInitialised = true;

        SeedGenerator(char* filename)
        {
            _filename = filename;
        };

        void showPrompt();

    public:

        static optional<SeedGenerator> Make(char* filename);
        
        void start(const uint8_t* randomSeed, uint8_t* master_seed, uint8_t* lightningMasterSeed);

        bool seedIsInitialised();

        uint8_t* getSeed();
};

#endif
