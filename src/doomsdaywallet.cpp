#include <iostream>

#include "features/random_generator.h"
#include "features/seed_generator.h"
#include "features/descriptors.h"
#include "input/input_mgr.h"
#include "bitcoin/wallets.h"

#define EXIT_CODE '9'

string walletIsReadyString(optional<WalletMgr> wallet) {
    string menu("");
    if (wallet.has_value()) {
        menu += " [" + wallet->getMasterFingerprint() + "] OK";
    }
    return menu;
}

void createMenu(optional<WalletMgr> wallet, optional<DescriptorMgr> descriptorMgr) {
    
    std::cout << "\033[2J\033[1;1H";
    std::cout << "* * * * * * * * * * * * * * *\n";
    std::cout << "*     Doomsday Wallet       *\n";
    std::cout << "* * * * * * * * * * * * * * *\n";
    std::cout << "\n";
    std::cout << "-- 1. Initialise master key" << walletIsReadyString(wallet) << endl;

    if (descriptorMgr.has_value() && descriptorMgr->getAccountKey().has_value()) {
        char descriptorOutput[MAX_DESCRIPTOR_LENGTH] = {'\0'};
        descriptorMgr->getAccountKey()->getDescriptor(descriptorOutput);
        std::cout << "-- 2. Reference account keys: " << descriptorOutput << endl;
    } else {
        std::cout << "-- 2. Generate account keys" << endl;
    }
    std::cout << "-- 3. List descriptors\n";
    std::cout << "-- 4. Export Lightning node private key\n";
    //std::cout << "-- 5. Export descriptors to Bitcoin full node\n";
    std::cout << "-- 9. Exit\n";
    std::cout << "Please select an option: ";
}

void randomSeedGeneration() {
    int input;
    InputMgr inputMgr = InputMgr::Make(1);
    do {
        std::cout << "\033[2J\033[1;1H";
        std::cout << "* * * * * * * * * * * * * * *\n";
        std::cout << "*     Doomsday Wallet       *\n";
        std::cout << "* * * * * * * * * * * * * * *\n";
        std::cout << endl;
        std::cout << generateRandomSeed();
        std::cout << endl << "Hit '1' to generate a new random seed.";
        std::cout << endl << "Hit '2' to exit." << endl;
        input = inputMgr.readChar();
    } while (input == '1');

    std::cout << endl << "This is the random seed, and it's been saved into your source code file features/seed.h" << endl;
    std::cout << "Now run 'make clean && make' in the terminal to build your Doomsday Wallet" << endl;
    std::cout << "ATTENTION: This IS NOT your private key, and it is safe to keep it stored as part of the source code." << endl;
    std::cout << "Just remember to store your source code." << endl;
}

void startGeneratingRandomSeed() {
    std::cout << "\033[2J\033[1;1H" << endl;
    std::cout << "* * * * * * * * * * * * * * *\n";
    std::cout << "*     Doomsday Wallet       *\n";
    std::cout << "* * * * * * * * * * * * * * *\n";
    std::cout << endl;
    std::cout << "Step 1: you must initialise your Doomsday Wallet with a pseudo random number." << endl;
    std::cout << "Hit enter to continue" << endl;
    getchar();
}

int main(int argc, char **argv) {
    SeedGenerator seedGenerator = SeedGenerator::Make(argv[1]);
    optional<DescriptorMgr> descriptorMgr = nullopt;
    optional<WalletMgr> wallet = nullopt;
    optional<WalletMgr> lightningWallet = nullopt;
        
    char choice = '\0';

    while (choice != EXIT_CODE) {
        if (!seedGenerator.randomSeedIsInitialised()) {
            startGeneratingRandomSeed();
            randomSeedGeneration();
            exit(0);
        } else {
            createMenu(wallet, descriptorMgr);
            choice = getchar();
            switch (choice) {
                case '1': {
                    uint8_t masterSeed[ENTROPY_SIZE] = {'\0'};
                    uint8_t lightningMasterSeed[ENTROPY_SIZE] = {'\0'};
                    
                    seedGenerator.start(masterSeed, lightningMasterSeed);
                    std::cout << "\n\nSeed generated, press return to continue..." << endl;
                    wallet = WalletMgr::Make("doomsday_wallet", masterSeed);
                    lightningWallet = WalletMgr::Make("lightning_wallet", lightningMasterSeed);
                    descriptorMgr = DescriptorMgr(masterSeed, ENTROPY_SIZE);                       
                    
                    getchar();
                    break;
                }
                case '2':
                    if (wallet.has_value()) {
                        int accountNumber = 0;
                        std::cout << "\n\nInsert an account number" << endl;
                        std::cin >> accountNumber;
                        wallet->generateAccountDescriptors(accountNumber);
                        char c = getchar();
                        std::cout << "\n\nAccount keys generated." << endl;
                    } else {
                        std::cout << "Master key must be initialised first.";
                    }
                    break;
                case '3':
                    if (wallet.has_value()) {
                        vector<string> descriptors = wallet->getDescriptors();
                        vector<string>::iterator it;
                        std::cout << endl;
                        for (it = descriptors.begin(); it != descriptors.end(); it++) {
                            std::cout << *it << endl;
                        }
                        char c = getchar();
                        std::cout << "\n\nNow copy the descriptors if you need to." << endl;
                        
                    } else {
                        std::cout << "Wallet must be initialised first." << endl;
                    }
                    break;
                case '4':
                    if (wallet.has_value()) {
                        Key masterLightningKey = lightningWallet->exportMasterPrivKey();
                        lightningWallet->generateAccountDescriptors(0);
                        std::cout << endl;
                        std::cout << "Lightning private key > " << masterLightningKey.exportXprivKey() << endl;
                        char c = getchar();
                        std::cout << endl;
                        std::cout << "This is the key to create a lightning node hot wallet." << endl;
                        std::cout << "Keep this key safe." << endl;
                        
                    } else {
                        std::cout << "Wallet must be initialised first." << endl;
                    }
                    break;
                case '5':
                /*
                    if (wallet.has_value()) {
                        wallet->getBlockchainInfo();
                        wallet->createWallet("xxxxxxxx", true);
                        vector<string> descriptors = wallet->getDescriptors();
                        vector<string>::iterator descriptor;
                        for (descriptor = descriptors.begin(); descriptor != descriptors.end(); descriptor++) {
                            optional<JsonObject> descriptorInfo = wallet->getDescriptorInfo(*descriptor);
                            if(descriptorInfo.has_value()) {
                                string descriptorImport = descriptorInfo->getChildAsString("descriptor");
                                string response = wallet->importDescriptor(descriptorImport, "test_import");
                            }
                        }
                        
                    } else {
                        std::cout << "Master key must be initialised first.";
                    }
                */
                    break;
            }
            std::cout << "\nPress return to continue... " << endl;
            char c = getchar();
            while (c != '\n') {
                std::cout << "Press return to continue... " << endl;
                c = getchar();
            }
        }
    }

    return 0;
}
