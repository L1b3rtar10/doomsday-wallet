#ifndef DESCRIPTOR_MGR_H
#define DESCRIPTOR_MGR_H

#include <optional>

#include "../crypto/key.h"

class DescriptorMgr
{    
    private:

        optional<Key> _masterKey = nullopt;
        optional<Key> _accountKey = nullopt;

        void showPrompt();

        int getUserInput();

    public:

        DescriptorMgr(uint8_t* masterSeed, uint8_t seedLength);

        void start();

        void generateAccountKey(int accountNumber);

        bool hasAccountKey();

        optional<Key> getAccountKey();

        void exportDescriptors();

};

#endif
