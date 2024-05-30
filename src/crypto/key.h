#ifndef CRYPTO_KEY
#define CRYPTO_KEY

#include <stdint.h>
#include <cstdlib>

#include "secp256k1.h"
#include "secp256k1_ecdh.h"

#define ENTROPY_SIZE 64
#define MASTER_HASH_KEY {'B','i','t','c','o','i','n',' ','s','e','e','d'}

// Generated from https://www.random.org/bytes/
#define RANDOM_INITIALISER {0xa9, 0x18, 0x6a, 0x26, 0x5e, 0xc9, 0xe8, 0x3c, 0x8e, 0x8a, 0x70, 0xc3,\
0x9a, 0xf0, 0xb6, 0xc0, 0x54, 0x27, 0x9b, 0x6e, 0x5b, 0x39, 0x49, 0x85, 0x64, 0x4d, 0xfe, 0x6e, 0x5e,\
0xdc, 0x37, 0x0a, 0x24, 0x20, 0xf0, 0xa9, 0x1e, 0xeb, 0x09, 0x43, 0x59, 0x30, 0xa4, 0xcd, 0x65, 0x18,\
0xd2, 0x0c, 0x71, 0xfa, 0x87, 0x29, 0x96, 0x1d, 0x73, 0x2a, 0xc9, 0x43, 0x5b, 0xce, 0xed, 0x00, 0x1b, 0x25}


#define MASTER_FINGERPRINT {0x00, 0x00, 0x00, 0x00}
#define MASTER_PARENT_SIGNATURE {0x00, 0x00, 0x00, 0x00}
#define MIN_HARDENED_CHILD_INDEX 2147483648

#define VERSION_BIP44_PRIV {0x04, 0x88, 0xAD, 0xE4}
#define VERSION_BIP44_PUB {0x04, 0x88, 0xB2, 0x1E}

#define VERSION_BIP84_PRIV {0x04, 0xB2, 0x43, 0x0C}
#define VERSION_BIP84_PUB {0x04, 0xB2, 0x47, 0x46}

#define SERIALIZED_KEY_LENGTH 120
#define MAX_DESCRIPTOR_LENGTH 255

// TEST VECTORS FROM https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki#test-vectors
#define TESTVECTOR_1 {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f}    
#define TESTVECTOR_2 {0xff,0xfc,0xf9,0xf6,0xf3,0xf0,0xed,0xea,0xe7,0xe4,0xe1,0xde,0xdb,0xd8,0xd5,0xd2,\
0xcf,0xcc,0xc9,0xc6,0xc3,0xc0,0xbd,0xba,0xb7,0xb4,0xb1,0xae,0xab,0xa8,0xa5,0xa2,0x9f,0x9c,0x99,0x96,0x93,\
0x90,0x8d,0x8a,0x87,0x84,0x81,0x7e,0x7b,0x78,0x75,0x72,0x6f,0x6c,0x69,0x66,0x63,0x60,0x5d,0x5a,0x57,0x54,\
0x51,0x4e,0x4b,0x48,0x45,0x42}


enum Keytype {
    BIP_44 = 44,
    BIP_49 = 49,
    BIP_84 = 84
};

enum AddressType {
    RECEIVING = 0,
    CHANGE = 1
};

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

        // DANGEROUS!
        Key(const uint8_t* data, const uint8_t* key, const size_t dataLength, const size_t keyLength, const Keytype keyType);

        secp256k1_pubkey& getPubKey();

        void initializePublicKey();

        void getPoint(const uint8_t* privateKey, uint8_t* outKey, size_t* size);

        void sumPrivKey(uint8_t* privKey, const uint8_t* number);

        static constexpr uint8_t masterKeyParentSignature[4] = MASTER_FINGERPRINT;

        void deriveChildKey(const uint32_t index, Key& outKey);

        void deriveChildXPubKey(const uint32_t index);

        void exportXpubKey(char* serialisedKey, size_t &serialisedLength);

        void exportXprivKey(char* serialisedKey, size_t& serialisedLength);

        void exportAccountKeys(Key& descriptorKey, uint32_t accountNumber);

        void exportAddressKeys(Key& descriptorKey, AddressType addressType);

        void getAddress(char* address);

        void debug();

        void exportDescriptor(char* descriptor, size_t &len, AddressType addressType);

        void erase();
};

#endif // CRYPTO_KEY
