#include <iostream>
#include <cassert>

#include "features/seed_generator.h"
#include "features/descriptors.h"
#include "bitcoin/wallets.h"
#include "bitcoin/jsonparser.h"

using namespace std;

// 64 random bytes
#define RANDOM_SEED { \
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  \
}

#define EXIT_CODE '6'

#define OPTIONS_MENU_NO_SEED "\n\n \
    +++ Choose an option +++\n\
    ----- 1. Initialise master key\n"


void createMenu(bool seedInitialised, optional<DescriptorMgr> descriptorMgr) {
    
    cout << "\033[2J\033[1;1H";
    cout << "* * * * * * * * * * * * * * *\n";
    cout << "*     Doomsday Wallet       *\n";
    cout << "* * * * * * * * * * * * * * *\n";
    cout << "\n";
    cout << "-- 1. Initialise master key" << (seedInitialised?" OK":"") << endl;

    if (descriptorMgr.has_value() && descriptorMgr->getAccountKey().has_value()) {
        char descriptorOutput[MAX_DESCRIPTOR_LENGTH] = {'\0'};
        descriptorMgr->getAccountKey()->getDescriptor(descriptorOutput);
        cout << "-- 2. Reference account keys: " << descriptorOutput << endl;
    } else {
        cout << "-- 2. Generate account keys" << endl;
    }
    cout << "-- 3. List descriptors\n";
    cout << "-- 4. Export Lightning node private key\n";
    cout << "-- 5. Export descriptors to Bitcoin full node\n";
    cout << "-- 6. Exit\n";
    cout << "Please select an option: ";
}

void printMenu() {
    cout << "\033[2J\033[1;1H";
    cout << OPTIONS_MENU_NO_SEED << "----- " << EXIT_CODE << ". Exit\n\n+++ Selection: ";
    return;
}

constexpr bool isAllZero(const uint8_t array[], size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (array[i] != 0) {
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv) {
    static constexpr uint8_t randomSeed[] = RANDOM_SEED;
    static_assert(sizeof(randomSeed) == ENTROPY_SIZE, "RANDOM_SEED provided MUST be 64 bytes long");
    static_assert(!isAllZero(randomSeed, ENTROPY_SIZE), "RANDOM_SEED provided should be generated randomly");
    
    optional<SeedGenerator> seedGenerator = SeedGenerator::Make(argv[1]);
    optional<DescriptorMgr> descriptorMgr = nullopt;
    optional<WalletMgr> wallet = nullopt;
    optional<WalletMgr> lightningWallet = nullopt;

    uint8_t testSeed[ENTROPY_SIZE] = {'\0'};
    wallet = WalletMgr::Make("test_watchonly", testSeed);

    createMenu(wallet.has_value(), descriptorMgr);
    
    char choice = '\0';

    while (choice != EXIT_CODE) {
        createMenu(wallet.has_value(), descriptorMgr);
        choice = getchar();
        switch (choice) {
            case '1':
                if (seedGenerator.has_value()) {
                    uint8_t masterSeed[ENTROPY_SIZE] = {'\0'};
                    uint8_t lightningMasterSeed[ENTROPY_SIZE] = {'\0'};
                    seedGenerator->start(randomSeed, masterSeed, lightningMasterSeed);
                    cout << "\n\nSeed generated, press return to continue..." << endl;
                    wallet = WalletMgr::Make("doomsday_wallet", masterSeed);
                    lightningWallet = WalletMgr::Make("lightning_wallet", lightningMasterSeed);
                    descriptorMgr = DescriptorMgr(masterSeed, ENTROPY_SIZE);
                }
                break;
            case '2':
                if (wallet.has_value()) {
                    int accountNumber = 0;
                    cout << "\n\nInsert an account number" << endl;
                    cin >> accountNumber;
                    wallet->generateAccountDescriptors(accountNumber);
                    char c = getchar();
                    cout << "\n\nAccount keys generated." << endl;
                } else {
                    cout << "Master key must be initialised first.";
                }
                break;
            case '3':
                if (wallet.has_value()) {
                    vector<string> descriptors = wallet->getDescriptors();
                    vector<string>::iterator it;
                    for (it = descriptors.begin(); it != descriptors.end(); it++) {
                        cout << *it << endl;
                    }
                    char c = getchar();
                    cout << "\n\nNow copy the descriptors if you need to." << endl;
                    
                } else {
                    cout << "Wallet must be initialised first." << endl;
                }
                break;
            case '4':
                if (wallet.has_value()) {
                    Key masterLightningKey = lightningWallet->exportMasterPrivKey();
                    Key masterKey = wallet->exportMasterPrivKey();

                    cout << "Lightning private key > " << masterLightningKey.exportXprivKey() << endl;
                    char c = getchar();
                    cout << endl;
                    cout << "This is the key to create a lightning node hot wallet." << endl;
                    cout << "Keep this key safe." << endl;
                    
                } else {
                    cout << "Wallet must be initialised first." << endl;
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
                    cout << "Master key must be initialised first.";
                }
            */
                break;
            
        }
        cout << "\nPress return to continue... " << endl;
        char c = getchar();
        while (c != '\n') {
            cout << "Press return to continue... " << endl;
            c = getchar();
        }
    }

    return 0;
}
