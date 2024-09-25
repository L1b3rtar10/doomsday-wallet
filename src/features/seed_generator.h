#ifndef SEED_GENERATOR_H
#define SEED_GENERATOR_H

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

        bool isAllZero(const uint8_t array[], size_t size);

    public:

        static SeedGenerator Make(char* filename);
        
        void start(uint8_t* master_seed, uint8_t* lightningMasterSeed);

        bool seedIsInitialised();

        uint8_t* getSeed();

        bool randomSeedIsInitialised();
};

#endif
