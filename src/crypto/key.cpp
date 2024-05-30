#include "key.h"

#include "secp256k1.h"
#include "secp256k1_ecdh.h"

#include "byteutils.h"
#include "byteutils.h"
#include "base58.h"
#include "sha-256.h"
#include "ripemd160.h"
#include "hmac_sha512.h"
#include "byteutils.h"

#include <cstdlib>
#include <assert.h>


Key::Key() {
    //printf("\nInit 3");
}

Key::Key(const uint8_t* data, const uint8_t* key, const size_t dataLength, const size_t keyLength,\
            const uint32_t index, const uint8_t depth, const uint8_t* parentFingerprint, const Keytype keyType,\
            bool isMaster, const char* parentDescriptor, const uint8_t descriptorLength) {

    _keyType = keyType;
    _index = index;
    _depth = depth;
    _isMaster = isMaster;
    memcpy(_parentFingerprint, parentFingerprint, 4);
    memcpy(keyMaterial, data, 32);
    memcpy(&keyMaterial[32], key, 32);
    memcpy(&keyMaterial[32], key, 32);
    buildKeyDescriptor(parentDescriptor, descriptorLength);
}


Key::Key(const uint8_t* data, const uint8_t* key, const size_t dataLength, const size_t keyLength, const Keytype keyType) : \
        Key(data, key, dataLength, keyLength, 0, 0, new uint8_t[4]{0x00, 0x00, 0x00, 0x00}, keyType, true, \
            new char[1]{'\0'}, 0) {
    
    /* Before we can call actual API functions, we need to create a "context". */
    ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);

    /* This is used to generate random private keys
    if (!fill_random(randomize, sizeof(randomize))) {
        printf("Failed to generate randomness\n");
        abort();
    }
    */
    /* Randomizing the context is recommended to protect against side-channel
     * leakage See `secp256k1_context_randomize` in secp256k1.h for more
     * information about it. This should never fail. */
    assert(secp256k1_context_randomize(ctx, randomize));

    CHMAC_SHA512{key, keyLength}.Write(data, dataLength).Finalize(keyMaterial);

    /* Public key creation using a valid context with a verified secret key should never fail */
    assert(secp256k1_ec_pubkey_create(ctx, &pubkey, keyMaterial));
}

secp256k1_pubkey& Key::getPubKey() {
    return pubkey;
}

void Key::getPoint(const uint8_t* privateKey, uint8_t* outKey, size_t* size) {
    secp256k1_pubkey tmp_pubKey;
    secp256k1_pubkey out_pubKey;

    secp256k1_pubkey* ins[2] = {&tmp_pubKey, &pubkey};
    
    // Creates a pubkey starting from the first 32 bites of privateKey
    assert(secp256k1_ec_pubkey_create(ctx, &tmp_pubKey, privateKey));
    
    assert(secp256k1_ec_pubkey_combine(ctx, &out_pubKey, ins, 2));

    uint8_t outputCompressed[33] = {0x00};
    size_t len = 33;

    secp256k1_ec_pubkey_serialize(ctx, outKey, size, &out_pubKey, SECP256K1_EC_COMPRESSED);
}

void Key::sumPrivKey(uint8_t* privKey, const uint8_t* number) {
    secp256k1_ec_seckey_tweak_add(ctx, privKey, number);
}


// Derives extended private child keys for normal and hardened children
void Key::deriveChildKey(const uint32_t index, Key& outKey) {
    
    uint8_t data[37] = {0x00}; // parent_pbkey(33 bytes) | index(4 bytes)

    uint8_t index_bytes[4] = {0x00}; 
    uint32ToBytes(index, index_bytes);

    uint8_t compressed_pubkey[33] = {0x00};
    size_t len = 33;
    secp256k1_ec_pubkey_serialize(ctx, compressed_pubkey, &len, &pubkey, SECP256K1_EC_COMPRESSED);
    assert(len == sizeof(compressed_pubkey));

    uint8_t child_chaincode[32] = {0x00};
    uint8_t vout[64] = {0x00};
    
    if (index < MIN_HARDENED_CHILD_INDEX) {
        memcpy(&data[0], compressed_pubkey, 33);
        memcpy(&data[33], index_bytes, 4);
        CHMAC_SHA512{&keyMaterial[32], 32}.Write(data, 37).Finalize(vout);
    } else {
        memcpy(&data[1], keyMaterial, 32);
        memcpy(&data[33], index_bytes, 4);
        CHMAC_SHA512{&keyMaterial[32], 32}.Write(data, 37).Finalize(vout);
    }

    memcpy(child_chaincode, &vout[32], 32);

    uint8_t child_priv_key[32] = {0x00};
    memcpy(child_priv_key, vout, 32);
    
    sumPrivKey(child_priv_key, keyMaterial);
    
    uint8_t parent_fingerprint[20] = {0x00};
    calculateKeyFingerprint(compressed_pubkey, parent_fingerprint);
    
    outKey = Key(child_priv_key, child_chaincode, sizeof(child_priv_key), sizeof(child_chaincode), index,\
                 _depth + 1, parent_fingerprint, _keyType, false, _descriptor, _descriptorLength);
    outKey.initializePublicKey();
}

void Key::deriveChildXPubKey(const uint32_t index) {
    assert(index < MIN_HARDENED_CHILD_INDEX);
    uint8_t data[37] = {0x00}; // parent_pbkey(33 bytes) | index(4 bytes)

    uint8_t index_bytes[4] = {0x00}; 
    uint32ToBytes(index, index_bytes);

    uint8_t compressed_pubkey[33] = {0x00};
    size_t size = 33;
    secp256k1_ec_pubkey_serialize(ctx, compressed_pubkey, &size, &pubkey, SECP256K1_EC_COMPRESSED);
    assert(size == sizeof(compressed_pubkey));

    uint8_t vout[64] = {0x00};
    
    memcpy(&data[0], compressed_pubkey, 33);
    memcpy(&data[33], index_bytes, 4);
    CHMAC_SHA512{&keyMaterial[32], 32}.Write(data, 37).Finalize(vout);

    uint8_t child_chaincode[32] = {0x00};
    memcpy(child_chaincode, &vout[32], 32);

    uint8_t child_priv_key[32] = {0x00};
    memcpy(child_priv_key, vout, 32);

    uint8_t child_pub_key[33] = {0x00};
    
    getPoint(child_priv_key, child_pub_key, &size);
    printf("\nChild pubkey is\n");
    print_string(child_pub_key, size);
    
    /*
    uint8_t parent_fingerprint[20] = {0x00};
    calculateKeyFingerprint(compressed_pubkey, parent_fingerprint);
    
    outKey = Key(child_priv_key, child_chaincode, sizeof(child_priv_key), sizeof(child_chaincode), index,\
                 _depth + 1, parent_fingerprint, _keyType, false, _descriptor, _descriptorLength);
    outKey.initializePublicKey();
    */
}

void Key::exportXprivKey(char* serialisedKey, size_t& serialisedLength) {
                        
    uint8_t PREFIX_LENGTH = 13;
    uint8_t version[4] = {0x00};
    getVersionCode(version, true);
    
    const uint8_t PARENT_FINGERPRINT_INDEX = 5;
    uint8_t CHILD_NUMBER[4] = {0x00};

    uint8_t checksum[32] = {0x00};

    char xpriv[SERIALIZED_KEY_LENGTH] = {'\0'};
    uint8_t input[82] = {0x00};

    memcpy(&input[PREFIX_LENGTH], &keyMaterial[32], 32);
    memcpy(&input[32 + 1 + PREFIX_LENGTH], keyMaterial, 32); // + 1 assuming key index is 0 at position 0

    memcpy(&input[0], version, 4);
    input[4] = _depth;
    memcpy(&input[PARENT_FINGERPRINT_INDEX], _parentFingerprint, 4);
    
    uint32ToBytes(_index, &input[9]);

    uint8_t tmp_checksum[32] = {0x00};
    calc_sha_256(tmp_checksum, &input, 78);
    memcpy(&input[78], tmp_checksum, 2);

    calc_sha_256(checksum, &tmp_checksum, 32);
    memcpy(&input[78], checksum, 4);

    EncodeBase58(input, 82, xpriv);

    size_t i = 0;

    for (i = 0; (i < SERIALIZED_KEY_LENGTH) && (xpriv[i] != '\0'); i++) {
        serialisedKey[i] = xpriv[i];
    }

    serialisedKey[i] = '\0';
    serialisedLength = i + 1;
}

// Calculates HASH160
void Key::calculateParentFingerprint(const uint8_t* parent_pbkey, uint8_t* parent_fingerprint) {
    unsigned char sha256Hash[SIZE_OF_SHA_256_HASH] = {0x00};

    calc_sha_256(sha256Hash, parent_pbkey, 33);
    CRIPEMD160{}.Write(sha256Hash, SIZE_OF_SHA_256_HASH).Finalize(parent_fingerprint);
}

void Key::debug() {
    printf("\nDEBUG!!!");
    printf("\nDebugging type > %d", _keyType);
    printf("\nDebugging key\n index > %d", _index);
    printf("\n depth > %d", _depth);
    printf("\n parent finger > ");
    print_string(_parentFingerprint, 4);
    printf("\n keymaterial > ");
    print_string(keyMaterial, 64);
    printf("\n");

    uint8_t compressed_pubkey[33] = {0x00};
    size_t len = 33;
    secp256k1_ec_pubkey_serialize(ctx, compressed_pubkey, &len, &pubkey, SECP256K1_EC_COMPRESSED);
    assert(len == sizeof(compressed_pubkey));
    printf("\n pubkey > ");
    print_string(compressed_pubkey, 33);
    printf("\n");
}

void Key::initializePublicKey() {
    ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
    assert(secp256k1_context_randomize(ctx, randomize));
    /* Public key creation using a valid context with a verified secret key should never fail */
    assert(secp256k1_ec_pubkey_create(ctx, &pubkey, keyMaterial));
}

void Key::exportXpubKey(char* serialisedKey, size_t &serialisedLength) {
    uint8_t PREFIX_LENGTH = 13;
    uint8_t version[4] = {0x00};
    getVersionCode(version, false);

    const uint8_t PARENT_FINGERPRINT_INDEX = 5;
    uint8_t CHILD_NUMBER[4] = {0x00};

    uint8_t checksum[32] = {0x00};

    char xpub[SERIALIZED_KEY_LENGTH] = {'\0'};
    uint8_t input[82] = {0x00};

    memcpy(&input[PREFIX_LENGTH], &keyMaterial[32], 32); // Copy chaincode

    const size_t COMPRESSED_KEY_SIZE = 33;
    size_t outputKeyLength = COMPRESSED_KEY_SIZE;
    uint8_t serialised_pubkey[COMPRESSED_KEY_SIZE] = {0x00};

    assert(secp256k1_ec_pubkey_serialize(ctx, serialised_pubkey, &outputKeyLength, &pubkey, SECP256K1_EC_COMPRESSED));
    assert(COMPRESSED_KEY_SIZE == outputKeyLength);

    memcpy(&input[32 + PREFIX_LENGTH], serialised_pubkey, 33);

    memcpy(&input[0], version, 4);
    input[4] = _depth;
    memcpy(&input[PARENT_FINGERPRINT_INDEX], _parentFingerprint, 4);
    
    uint32ToBytes(_index, &input[9]);
    
    uint8_t tmp_checksum[32] = {0x00};
    calc_sha_256(tmp_checksum, &input, 78);
    memcpy(&input[78], tmp_checksum, 2);

    calc_sha_256(checksum, &tmp_checksum, 32);
    memcpy(&input[78], checksum, 4);

    EncodeBase58(input, 82, xpub);

    size_t i = 0;

    for (i = 0; (i < SERIALIZED_KEY_LENGTH) && (xpub[i] != '\0'); i++) {
        serialisedKey[i] = xpub[i];
    }

    serialisedKey[i] = '\0';
    serialisedLength = i + 1;
}

/*** Version code should be different, but importdescriptor won't work with zpriv and zpub keys ***/
/*** This method will return keys in the format xpriv/xpub for BIP84 keys ***/
/*** Uncomment to have the real BIP84 format ***/
void Key::getVersionCode(uint8_t* versionCode, bool isPrivate) {

    if (isPrivate) {
        //switch (_keyType) {
        //    case BIP_44: {
                uint8_t version[4] = VERSION_BIP44_PRIV;
                memcpy(versionCode, version, 4);
        //    } break;

        /*    case BIP_84: {
                uint8_t version[4] = VERSION_BIP84_PRIV;
                memcpy(versionCode, version, 4);
            } break;

            default:
                break;
        }*/
    } else {
        //switch (_keyType) {
        //    case BIP_44: {
                uint8_t version[4] = VERSION_BIP44_PUB;
                memcpy(versionCode, version, 4);
        /*    } break;

            case BIP_84: {
                uint8_t version[4] = VERSION_BIP84_PUB;
                memcpy(versionCode, version, 4);
            } break;

            default:
                break;
        }*/    
    }
}

// https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki#examples
// 84h/ -> https://en.bitcoin.it/wiki/BIP_0084
void Key::exportAccountKeys(Key& accountKey, uint32_t accountNumber) {
    Key purposeKey = Key();
    deriveChildKey(MIN_HARDENED_CHILD_INDEX + _keyType, purposeKey);

    Key coinTypeKey = Key(); //0h/ https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki#examples
    purposeKey.deriveChildKey(MIN_HARDENED_CHILD_INDEX, coinTypeKey);

    coinTypeKey.deriveChildKey(MIN_HARDENED_CHILD_INDEX + accountNumber, accountKey);
}

void Key::exportAddressKeys(Key& addressKey, AddressType addressType) {
    Key key = Key();
    deriveChildKey(addressType, addressKey);
}

void Key::buildKeyDescriptor(const char* parentDescriptor, const size_t length) {

    char indexString[10] = {'\0'};

    if (_depth <= 1) {
        _descriptor[0] = '[';
        _descriptorLength++;

        for (size_t i = 0; i < 4; ++i) {
            byteToHex(_parentFingerprint[i], &_descriptor[(i * 2) + 1]);
            _descriptorLength += 2;
        }
    } else {
        memcpy(&_descriptor[_descriptorLength], parentDescriptor, length);
        _descriptorLength += length;
    }
    
    _descriptor[_descriptorLength++] = '/';
    
    size_t indexLength = 0;
    uint32_t indexNormalised = 0;
    
    if (_depth > 0) {
        if (_index >= MIN_HARDENED_CHILD_INDEX) {
            indexNormalised = _index - MIN_HARDENED_CHILD_INDEX;
        }
        uint32ToDecimalString(indexNormalised, indexString, indexLength);
        memcpy(&_descriptor[_descriptorLength], indexString, indexLength);
        _descriptorLength+=indexLength;
        if (_index >= MIN_HARDENED_CHILD_INDEX) {
            _descriptor[_descriptorLength++] = 'h';
        }
    }

    char desc[MAX_DESCRIPTOR_LENGTH] = {'\0'};
    size_t keyLength = 0;
}

void Key::exportDescriptor(char* descriptor, size_t &len, AddressType addressType) {
    char SUFFIX[5] = "/0/*";

    memcpy(descriptor, _descriptor, _descriptorLength);

    uint8_t index = _descriptorLength;
    descriptor[index++] = ']';

    size_t keyLength = 0;
    exportXprivKey(&descriptor[index], keyLength);
    index += keyLength;

    SUFFIX[1] = addressTypeToChar(addressType);
    memcpy(&descriptor[index], SUFFIX, sizeof(SUFFIX));
    index += sizeof(SUFFIX);
    len = index;
}

/* Cleanses memory to prevent leaking sensitive info. Won't be optimized out. */
void Key::secureErase(void *ptr, size_t len) {
#if defined(_MSC_VER)
    /* SecureZeroMemory is guaranteed not to be optimized out by MSVC. */
    SecureZeroMemory(ptr, len);
#elif defined(__GNUC__)
    /* We use a memory barrier that scares the compiler away from optimizing out the memset.
     *
     * Quoting Adam Langley <agl@google.com> in commit ad1907fe73334d6c696c8539646c21b11178f20f
     * in BoringSSL (ISC License):
     *    As best as we can tell, this is sufficient to break any optimisations that
     *    might try to eliminate "superfluous" memsets.
     * This method used in memzero_explicit() the Linux kernel, too. Its advantage is that it is
     * pretty efficient, because the compiler can still implement the memset() efficiently,
     * just not remove it entirely. See "Dead Store Elimination (Still) Considered Harmful" by
     * Yang et al. (USENIX Security 2017) for more background.
     */
    memset(ptr, 0, len);
    __asm__ __volatile__("" : : "r"(ptr) : "memory");
#else
    void *(*volatile const volatile_memset)(void *, int, size_t) = memset;
    volatile_memset(ptr, 0, len);
#endif
}

void Key::erase() {
    secp256k1_context_destroy(ctx);
    secureErase(keyMaterial, sizeof(keyMaterial));
    secureErase(_parentFingerprint, sizeof(_parentFingerprint));
    secureErase(&pubkey, sizeof(pubkey));
}

char Key::addressTypeToChar(AddressType value) {
    return (value == RECEIVING) ? '0' : '1';
}

void Key::getAddress(char* address) {
    uint8_t compressed_pubkey[33] = {0x00};
    size_t len = 33;
    secp256k1_ec_pubkey_serialize(ctx, compressed_pubkey, &len, &pubkey, SECP256K1_EC_COMPRESSED);
    assert(len == sizeof(compressed_pubkey));

    generateP2WPKHAddress(compressed_pubkey, address);
}
