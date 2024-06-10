#include "descriptors.h"

#include <iostream>
#include <limits>
#include <string>

#include "../crypto/key.h"

DescriptorMgr::DescriptorMgr(uint8_t* masterSeed, uint8_t seedLength)
{
    _masterKey = Key::MakeMasterKey(masterSeed, seedLength);
};

void DescriptorMgr::start()
{
    showPrompt();
    int accountNumber = getUserInput();
    generateAccountKey(accountNumber);
    getchar();
}

void DescriptorMgr::generateAccountKey(int accountNumber)
{
    Key accountKey = Key();
    size_t descriptorLength = 0;

    char xpriv_account_receiving_descriptor[MAX_DESCRIPTOR_LENGTH] = {'\0'};
    if (_masterKey.has_value()) {
        _masterKey->exportAccountKeys(accountKey, accountNumber);
        _accountKey = accountKey;
    }
    
    cout << "\nAccount key generated" << endl;
}

bool DescriptorMgr::hasAccountKey()
{
    return _accountKey.has_value();
}

optional<Key> DescriptorMgr::getAccountKey()
{
    return _accountKey;
}

void DescriptorMgr::exportDescriptors(Keytype Keytype, AddressType addressType, char* descriptor)
{
    if (_accountKey.has_value()) {
        Key chainKey;
        _accountKey->deriveChildKey(BIP_44, chainKey);
        chainKey.exportDescriptor(descriptor, addressType);
    }
}

int DescriptorMgr::getUserInput() {
    int userInput;
    while (true) {
        std::cout << "Enter an integer value: ";
        std::cin >> userInput;

        if (std::cin.fail()) {
            // Clear the error flag on std::cin
            std::cin.clear();
            // Ignore the incorrect input
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter an integer value." << std::endl;
        } else {
            // Check if there are remaining characters in the buffer
            char remainingChar;
            std::cin.get(remainingChar);
            if (remainingChar == '\n') {
                return userInput; // Valid input, no extra characters
            } else {
                // Invalid input due to extra characters
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Please enter an integer value." << std::endl;
            }
        }
    }
}

void DescriptorMgr::showPrompt()
{
    cout << "\033[2J\033[1;1H";
    cout << "* * * * * * * * * * * * * \n";
    cout << "*  Generate account key * \n";
    cout << "* * * * * * * * * * * * * \n";
}
