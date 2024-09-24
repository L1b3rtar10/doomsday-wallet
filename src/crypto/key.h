#ifndef CRYPTO_KEY
#define CRYPTO_KEY

#include <iostream>
#include <stdint.h>
#include <string.h>

#include "secp256k1_ecdh.h"

#define ENTROPY_SIZE 64
#define MASTER_HASH_KEY {'B','i','t','c','o','i','n',' ','s','e','e','d'}

#define MASTER_FINGERPRINT {0x00, 0x00, 0x00, 0x00}
#define MASTER_PARENT_SIGNATURE {0x00, 0x00, 0x00, 0x00}
#define MIN_HARDENED_CHILD_INDEX 2147483648

#define VERSION_BIP44_PRIV {0x04, 0x88, 0xAD, 0xE4}
#define VERSION_BIP44_PUB {0x04, 0x88, 0xB2, 0x1E}

#define VERSION_BIP49_PRIV {0x04, 0x9D, 0x78, 0x78}
#define VERSION_BIP49_PUB {0x04, 0x9D, 0x7C, 0xB2}

#define VERSION_BIP84_PRIV {0x04, 0xB2, 0x43, 0x0C}
#define VERSION_BIP84_PUB {0x04, 0xB2, 0x47, 0x46}

#define SERIALIZED_KEY_LENGTH 111
#define MAX_DESCRIPTOR_LENGTH 255

enum Keytype {
    BIP_44 = 44,
    BIP_49 = 49,
    BIP_84 = 84,
    BIP_86 = 86
};

enum AddressType {
    RECEIVING = 0,
    CHANGE = 1
};

using namespace std;

/** A utility class for secp256k1 operations. */
class Key {

    private:
        Keytype _keyType = BIP_84;
        uint8_t _depth = 0x00;
        uint32_t _index = 0;
        bool _isMaster = false;
        uint8_t _parentFingerprint[4] = {0x00};
        uint8_t randomize[32] = {0x00};
        secp256k1_context* ctx;
        uint8_t keyMaterial[ENTROPY_SIZE] = {0x00}; // CHAINCODE[32] || KEY[32]
        secp256k1_pubkey pubkey;

        char _descriptor[MAX_DESCRIPTOR_LENGTH] = {'\0'};
        uint8_t _descriptorLength = 0;

        // DANGEROUS!
        Key(const uint8_t* data, const uint8_t* key, const size_t dataLength, const size_t keyLength, const Keytype keyType);

        void calculateParentFingerprint(const uint8_t* parent_pbkey, uint8_t* parent_fingerprint);

        void buildKeyDescriptor(const char* parentDescriptor, const size_t length);

        void getVersionCode(uint8_t* versionCode, bool isPrivate);

        void secureErase(void *ptr, size_t len);

        char addressTypeToChar(AddressType value);

    public:

        Key();

        Key(const uint8_t* data, const uint8_t* key, const size_t dataLength, const size_t keyLength,\
            const uint32_t index, const uint8_t depth, const uint8_t* parentFingerprint, const Keytype keyType,
            bool isMaster, const char* parentDescriptor, const uint8_t descriptorLength);

        static Key MakeMasterKey(uint8_t *masterSeed, uint8_t seedLength);

        secp256k1_pubkey& getPubKey();

        void initializePublicKey();

        void getPoint(const uint8_t* privateKey, uint8_t* outKey, size_t* size);

        void sumPrivKey(uint8_t* privKey, const uint8_t* number);

        static constexpr uint8_t masterKeyParentSignature[4] = MASTER_FINGERPRINT;

        void exportXpubKey(char* serialisedKey, size_t &serialisedLength);

        const string exportXprivKey();

        void deriveChildKey(const uint32_t index, Key& outKey);

        void exportAccountKeys(Key& descriptorKey, uint32_t accountNumber);

        void exportAddressKeys(Key& descriptorKey, AddressType addressType);

        const string getAddress();

        void debug();

        void generateBipChainKey(Keytype keyType, Key chainKey);

        void getDescriptor(char* descriptor);

        void exportDescriptor(char* descriptor);

        void exportAddressDescriptor(Keytype keyType, uint32_t accountNumber, AddressType addressType, char* descriptor);

        void erase();
};

#endif // CRYPTO_KEY
