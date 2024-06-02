#ifndef SEED_GENERATOR_H
#define SEED_GENERATOR_H

#define BUF_LEN 256
#define KEY_SIZE 64

#include <optional>

using namespace std;


class SeedGenerator
{    
    private:

        char* _filename;
        uint8_t* _randomSeed;
        bool _seedInitialised = false;

        SeedGenerator(char* filename, uint8_t* randomSeed)
        {
            _filename = filename;
            _randomSeed = randomSeed;
        };

        void showPrompt();

    public:

        static std::optional<SeedGenerator> Make(char* filename, uint8_t* randomSeed);
        
        void start(uint8_t* entropy);

        bool seedIsInitialised();
};

#endif
