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

void DescriptorMgr::exportDescriptors()
{
    char descriptor[MAX_DESCRIPTOR_LENGTH] = {'\0'};
    showPrompt();
    uint32_t accountNumber = getUserInput();

    if (_masterKey.has_value()) {
        _masterKey->exportAddressDescriptor(BIP_44, accountNumber, RECEIVING, descriptor);
        cout << descriptor << endl;
        memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

        _masterKey->exportAddressDescriptor(BIP_44, accountNumber, CHANGE, descriptor);
        cout << descriptor << endl;
        memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

         _masterKey->exportAddressDescriptor(BIP_49, accountNumber, RECEIVING, descriptor);
        cout << descriptor << endl;
        memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

        _masterKey->exportAddressDescriptor(BIP_49, accountNumber, CHANGE, descriptor);
        cout << descriptor << endl;
        memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

        _masterKey->exportAddressDescriptor(BIP_84, accountNumber, RECEIVING, descriptor);
        cout << descriptor << endl;
        memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

        _masterKey->exportAddressDescriptor(BIP_84, accountNumber, CHANGE, descriptor);
        cout << descriptor << endl;
        memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

         _masterKey->exportAddressDescriptor(BIP_86, accountNumber, RECEIVING, descriptor);
        cout << descriptor << endl;
        memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

        _masterKey->exportAddressDescriptor(BIP_86, accountNumber, CHANGE, descriptor);
        cout << descriptor << endl;
        memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);
    }

    getchar();
}

int DescriptorMgr::getUserInput() {
    int userInput;
    while (true) {
        std::cout << "Enter account number: ";
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
