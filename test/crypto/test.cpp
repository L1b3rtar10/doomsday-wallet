#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

#include "../../secp256k1/include/secp256k1.h"
#include "../../secp256k1/include/secp256k1_ecdh.h"

#include "../../src/crypto/sha-256.h"
#include "../../src/crypto/sha-256.cpp"
#include "../../src/crypto/sha512.h"
#include "../../src/crypto/sha512.cpp"
#include "../../src/crypto/hmac_sha512.h"
#include "../../src/crypto/hmac_sha512.cpp"
#include "../../src/crypto/ripemd160.h"
#include "../../src/crypto/ripemd160.cpp"
#include "../../src/crypto/bech32.h"
#include "../../src/crypto/bech32.cpp"
#include "../../src/crypto/base58.h"
#include "../../src/crypto/base58.cpp"
#include "../../src/crypto/byteutils.h"
#include "../../src/crypto/byteutils.cpp"
#include "../../src/crypto/key.h"
#include "../../src/crypto/key.cpp"
#include "../../src/features/descriptors.h"
#include "../../src/features/descriptors.cpp"

// TEST VECTORS FROM https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki#test-vectors
#define TEST_VECTOR_1 {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f}    

#define BOOST_TEST_MODULE Main
#define PARSER_TESTS crypto_tests

#include <boost/test/included/unit_test.hpp>
using namespace std;
using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( Crypto)

static void segwit_scriptpubkey(uint8_t* scriptpubkey, size_t* scriptpubkeylen, int witver, const uint8_t* witprog, size_t witprog_len) {
    scriptpubkey[0] = witver ? (0x50 + witver) : 0;
    scriptpubkey[1] = witprog_len;
    memcpy(scriptpubkey + 2, witprog, witprog_len);
    *scriptpubkeylen = witprog_len + 2;
}

int my_strncasecmp(const char *s1, const char *s2, size_t n) {
    size_t i = 0;
    while (i < n) {
        char c1 = s1[i];
        char c2 = s2[i];
        if (c1 >= 'A' && c1 <= 'Z') c1 = (c1 - 'A') + 'a';
        if (c2 >= 'A' && c2 <= 'Z') c2 = (c2 - 'A') + 'a';
        if (c1 < c2) return -1;
        if (c1 > c2) return 1;
        if (c1 == 0) return 0;
        ++i;
    }
    return 0;
}

BOOST_AUTO_TEST_CASE( bech32 )
{
    static const char* valid_checksum_bech32[] = {
        "A12UEL5L",
        "a12uel5l",
        "an83characterlonghumanreadablepartthatcontainsthenumber1andtheexcludedcharactersbio1tt5tgs",
        "abcdef1qpzry9x8gf2tvdw0s3jn54khce6mua7lmqqqxw",
        "11qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqc8247j",
        "split1checkupstagehandshakeupstreamerranterredcaperred2y9e3w",
        "?1ezyfcl",
    };

    static const char* valid_checksum_bech32m[] = {
        "A1LQFN3A",
        "a1lqfn3a",
        "an83characterlonghumanreadablepartthatcontainsthetheexcludedcharactersbioandnumber11sg7hg6",
        "abcdef1l7aum6echk45nj3s0wdvt2fg8x9yrzpqzd3ryx",
        "11llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllludsr8",
        "split1checkupstagehandshakeupstreamerranterredcaperredlc445v",
        "?1v759aa",
    };

    static const char* invalid_checksum_bech32[] = {
        " 1nwldj5",
        "\x7f""1axkwrx",
        "\x80""1eym55h",
        "an84characterslonghumanreadablepartthatcontainsthenumber1andtheexcludedcharactersbio1569pvx",
        "pzry9x0s0muk",
        "1pzry9x0s0muk",
        "x1b4n0q5v",
        "li1dgmt3",
        "de1lg7wt\xff",
        "A1G7SGD8",
        "10a06t8",
        "1qzzfhee",
    };

    static const char* invalid_checksum_bech32m[] = {
        " 1xj0phk",
        "\x7F""1g6xzxy",
        "\x80""1vctc34",
        "an84characterslonghumanreadablepartthatcontainsthetheexcludedcharactersbioandnumber11d6pts4",
        "qyrz8wqd2c9m",
        "1qyrz8wqd2c9m",
        "y1b0jsk6g",
        "lt1igcx5c0",
        "in1muywd",
        "mm1crxm3i",
        "au1s5cgom",
        "M1VUXWEZ",
        "16plkw9",
        "1p2gdwpf",
    };

    struct valid_address_data {
        const char* address;
        size_t scriptPubKeyLen;
        const uint8_t scriptPubKey[42];
    };

    struct invalid_address_data {
        const char* hrp;
        int version;
        size_t program_length;
    };

    static struct valid_address_data valid_address[] = {
        {
            "BC1QW508D6QEJXTDG4Y5R3ZARVARY0C5XW7KV8F3T4",
            22, {
                0x00, 0x14, 0x75, 0x1e, 0x76, 0xe8, 0x19, 0x91, 0x96, 0xd4, 0x54,
                0x94, 0x1c, 0x45, 0xd1, 0xb3, 0xa3, 0x23, 0xf1, 0x43, 0x3b, 0xd6
            }
        },
        {
            "tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3q0sl5k7",
            34, {
                0x00, 0x20, 0x18, 0x63, 0x14, 0x3c, 0x14, 0xc5, 0x16, 0x68, 0x04,
                0xbd, 0x19, 0x20, 0x33, 0x56, 0xda, 0x13, 0x6c, 0x98, 0x56, 0x78,
                0xcd, 0x4d, 0x27, 0xa1, 0xb8, 0xc6, 0x32, 0x96, 0x04, 0x90, 0x32,
                0x62
            }
        },
        {
            "bc1pw508d6qejxtdg4y5r3zarvary0c5xw7kw508d6qejxtdg4y5r3zarvary0c5xw7kt5nd6y",
            42, {
                0x51, 0x28, 0x75, 0x1e, 0x76, 0xe8, 0x19, 0x91, 0x96, 0xd4, 0x54,
                0x94, 0x1c, 0x45, 0xd1, 0xb3, 0xa3, 0x23, 0xf1, 0x43, 0x3b, 0xd6,
                0x75, 0x1e, 0x76, 0xe8, 0x19, 0x91, 0x96, 0xd4, 0x54, 0x94, 0x1c,
                0x45, 0xd1, 0xb3, 0xa3, 0x23, 0xf1, 0x43, 0x3b, 0xd6
            }
        },
        {
            "BC1SW50QGDZ25J",
            4, {
            0x60, 0x02, 0x75, 0x1e
            }
        },
        {
            "bc1zw508d6qejxtdg4y5r3zarvaryvaxxpcs",
            18, {
                0x52, 0x10, 0x75, 0x1e, 0x76, 0xe8, 0x19, 0x91, 0x96, 0xd4, 0x54,
                0x94, 0x1c, 0x45, 0xd1, 0xb3, 0xa3, 0x23
            }
        },
        {
            "tb1qqqqqp399et2xygdj5xreqhjjvcmzhxw4aywxecjdzew6hylgvsesrxh6hy",
            34, {
                0x00, 0x20, 0x00, 0x00, 0x00, 0xc4, 0xa5, 0xca, 0xd4, 0x62, 0x21,
                0xb2, 0xa1, 0x87, 0x90, 0x5e, 0x52, 0x66, 0x36, 0x2b, 0x99, 0xd5,
                0xe9, 0x1c, 0x6c, 0xe2, 0x4d, 0x16, 0x5d, 0xab, 0x93, 0xe8, 0x64,
                0x33
            }
        },
        {
            "tb1pqqqqp399et2xygdj5xreqhjjvcmzhxw4aywxecjdzew6hylgvsesf3hn0c",
            34, {
                0x51, 0x20, 0x00, 0x00, 0x00, 0xc4, 0xa5, 0xca, 0xd4, 0x62, 0x21,
                0xb2, 0xa1, 0x87, 0x90, 0x5e, 0x52, 0x66, 0x36, 0x2b, 0x99, 0xd5,
                0xe9, 0x1c, 0x6c, 0xe2, 0x4d, 0x16, 0x5d, 0xab, 0x93, 0xe8, 0x64,
                0x33
            }
        },
        {
            "bc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqzk5jj0",
            34, {
                0x51, 0x20, 0x79, 0xbe, 0x66, 0x7e, 0xf9, 0xdc, 0xbb, 0xac, 0x55,
                0xa0, 0x62, 0x95, 0xce, 0x87, 0x0b, 0x07, 0x02, 0x9b, 0xfc, 0xdb,
                0x2d, 0xce, 0x28, 0xd9, 0x59, 0xf2, 0x81, 0x5b, 0x16, 0xf8, 0x17,
                0x98
            }
        },
    };

    static const char* invalid_address[] = {
        "tc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vq5zuyut",
        "bc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqh2y7hd",
        "tb1z0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqglt7rf",
        "BC1S0XLXVLHEMJA6C4DQV22UAPCTQUPFHLXM9H8Z3K2E72Q4K9HCZ7VQ54WELL",
        "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kemeawh",
        "tb1q0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vq24jc47",
        "bc1p38j9r5y49hruaue7wxjce0updqjuyyx0kh56v8s25huc6995vvpql3jow4",
        "BC130XLXVLHEMJA6C4DQV22UAPCTQUPFHLXM9H8Z3K2E72Q4K9HCZ7VQ7ZWS8R",
        "bc1pw5dgrnzv",
        "bc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7v8n0nx0muaewav253zgeav",
        "BC1QR508D6QEJXTDG4Y5R3ZARVARYV98GJ9P",
        "tb1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vq47Zagq",
        "bc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7v07qwwzcrf",
        "tb1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vpggkg4j",
        "bc1gmk9yu",
    };

    static struct invalid_address_data invalid_address_enc[] = {
        {"BC", 0, 20},
        {"bc", 0, 21},
        {"bc", 17, 32},
        {"bc", 1, 1},
        {"bc", 16, 41},
    };

    size_t i;
    int fail = 0;
    for (i = 0; i < sizeof(valid_checksum_bech32) / sizeof(valid_checksum_bech32[0]); ++i) {
        uint8_t data[82];
        char rebuild[92];
        char hrp[84];
        size_t data_len;
        int ok = 1;
        if (bech32_decode(hrp, data, &data_len, valid_checksum_bech32[i]) != BECH32_ENCODING_BECH32) {
            printf("bech32_decode fails: '%s'\n", valid_checksum_bech32[i]);
            ok = 0;
        }
        if (ok) {
            if (!bech32_encode(rebuild, hrp, data, data_len, BECH32_ENCODING_BECH32)) {
                printf("bech32_encode fails: '%s'\n", valid_checksum_bech32[i]);
                ok = 0;
            }
        }
        if (ok && my_strncasecmp(rebuild, valid_checksum_bech32[i], 92)) {
            printf("bech32_encode produces incorrect result: '%s'\n", valid_checksum_bech32[i]);
            ok = 0;
        }
        fail += !ok;
    }
    for (i = 0; i < sizeof(invalid_checksum_bech32) / sizeof(invalid_checksum_bech32[0]); ++i) {
        uint8_t data[82];
        char hrp[84];
        size_t data_len;
        int ok = 1;
        if (bech32_decode(hrp, data, &data_len, invalid_checksum_bech32[i]) == BECH32_ENCODING_BECH32) {
            printf("bech32_decode succeeds on invalid string: '%s'\n", invalid_checksum_bech32[i]);
            ok = 0;
        }
        fail += !ok;
    }
    for (i = 0; i < sizeof(valid_checksum_bech32m) / sizeof(valid_checksum_bech32m[0]); ++i) {
        uint8_t data[82];
        char rebuild[92];
        char hrp[84];
        size_t data_len;
        int ok = 1;
        if (bech32_decode(hrp, data, &data_len, valid_checksum_bech32m[i]) != BECH32_ENCODING_BECH32M) {
            printf("bech32_decode fails: '%s'\n", valid_checksum_bech32m[i]);
            ok = 0;
        }
        if (ok) {
            if (!bech32_encode(rebuild, hrp, data, data_len, BECH32_ENCODING_BECH32M)) {
                printf("bech32_encode fails: '%s'\n", valid_checksum_bech32m[i]);
                ok = 0;
            }
        }
        if (ok && my_strncasecmp(rebuild, valid_checksum_bech32m[i], 92)) {
            printf("bech32_encode produces incorrect result: '%s'\n", valid_checksum_bech32m[i]);
            ok = 0;
        }
        fail += !ok;
    }
    for (i = 0; i < sizeof(invalid_checksum_bech32m) / sizeof(invalid_checksum_bech32m[0]); ++i) {
        uint8_t data[82];
        char hrp[84];
        size_t data_len;
        int ok = 1;
        if (bech32_decode(hrp, data, &data_len, invalid_checksum_bech32m[i]) == BECH32_ENCODING_BECH32M) {
            printf("bech32_decode succeeds on invalid string: '%s'\n", invalid_checksum_bech32m[i]);
            ok = 0;
        }
        fail += !ok;
    }
    for (i = 0; i < sizeof(valid_address) / sizeof(valid_address[0]); ++i) {
        uint8_t witprog[40];
        size_t witprog_len;
        int witver;
        const char* hrp = "bc";
        int ok = 1;
        uint8_t scriptpubkey[42];
        size_t scriptpubkey_len;
        char rebuild[93];
        int ret = segwit_addr_decode(&witver, witprog, &witprog_len, hrp, valid_address[i].address);
        if (!ret) {
            hrp = "tb";
            ret = segwit_addr_decode(&witver, witprog, &witprog_len, hrp, valid_address[i].address);
        }
        if (!ret) {
            printf("segwit_addr_decode fails: '%s'\n", valid_address[i].address);
            ok = 0;
        }
        if (ok) segwit_scriptpubkey(scriptpubkey, &scriptpubkey_len, witver, witprog, witprog_len);
        if (ok && (scriptpubkey_len != valid_address[i].scriptPubKeyLen || memcmp(scriptpubkey, valid_address[i].scriptPubKey, scriptpubkey_len))) {
            printf("segwit_addr_decode produces wrong result: '%s'\n", valid_address[i].address);
            ok = 0;
        }
        if (ok && !segwit_addr_encode(rebuild, hrp, witver, witprog, witprog_len)) {
            printf("segwit_addr_encode fails: '%s'\n", valid_address[i].address);
            ok = 0;
        }
        if (ok && my_strncasecmp(valid_address[i].address, rebuild, 93)) {
            printf("segwit_addr_encode produces wrong result: '%s'\n", valid_address[i].address);
            ok = 0;
        }
        fail += !ok;
    }
    for (i = 0; i < sizeof(invalid_address) / sizeof(invalid_address[0]); ++i) {
        uint8_t witprog[40];
        size_t witprog_len;
        int witver;
        int ok = 1;
        if (segwit_addr_decode(&witver, witprog, &witprog_len, "bc", invalid_address[i])) {
            printf("segwit_addr_decode succeeds on invalid address '%s'\n", invalid_address[i]);
            ok = 0;
        }
        if (segwit_addr_decode(&witver, witprog, &witprog_len, "tb", invalid_address[i])) {
            printf("segwit_addr_decode succeeds on invalid address '%s'\n", invalid_address[i]);
            ok = 0;
        }
        fail += !ok;
    }
    for (i = 0; i < sizeof(invalid_address_enc) / sizeof(invalid_address_enc[0]); ++i) {
        char rebuild[93];
        static const uint8_t program[42] = {0};
        if (segwit_addr_encode(rebuild, invalid_address_enc[i].hrp, invalid_address_enc[i].version, program, invalid_address_enc[i].program_length)) {
            printf("segwit_addr_encode succeeds on invalid input '%s'\n", rebuild);
            ++fail;
        }
    }
    printf("%i failures\n", fail);
    
    BOOST_CHECK(0 == fail);
}

BOOST_AUTO_TEST_CASE( generateAddress ) {

    char TEST_ADDRESS[] = "bc1q4uy58pzv8gfr6qq9dgtcjp93pe9n37mjclmz6a";
    char address[43] = {'\0'};
    const uint8_t publicKey[] = {
        0x02, 0x0c, 0xc5, 0xc7, 0xd8, 0x33, 0xcf, 0xb2, 
        0x35, 0x50, 0xf6, 0x03, 0x12, 0x27, 0xc5, 0x2b, 
        0x95, 0x9b, 0x04, 0xf5, 0xfd, 0x4e, 0xaf, 0x24, 
        0x30, 0x11, 0x91, 0xba, 0x8b, 0x3f, 0x67, 0xd0, 
        0xfa
    };

    generateP2WPKHAddress(publicKey, address);
    for (size_t i = 0; i < sizeof(TEST_ADDRESS); i++) {
        BOOST_CHECK(TEST_ADDRESS[i] == address[i]);
    }
}

BOOST_AUTO_TEST_CASE( encodeWIFKey ) {
    char TEST_KEY[] = "5HueCGU8rMjxEXxiPuD5BDku4MkFqeZyd4dZ1jvhTVqvbTLvyTJ";
    char wifKey[52] = {'\0'};
    const uint8_t data[] = {
        0x0C, 0x28, 0xFC, 0xA3, 0x86, 0xC7, 0xA2, 0x27,
        0x60, 0x0B, 0x2F, 0xE5, 0x0B, 0x7C, 0xAE, 0x11,
        0xEC, 0x86, 0xD3, 0xBF, 0x1F, 0xBE, 0x47, 0x1B,
        0xE8, 0x98, 0x27, 0xE1, 0x9D, 0x72, 0xAA, 0x1D
    };

    exportWIFKey(data, 32, wifKey);
    for (size_t i = 0; i < sizeof(TEST_KEY); i++) {
        BOOST_CHECK(TEST_KEY[i] == wifKey[i]);
    }
}

BOOST_AUTO_TEST_CASE( BIP44 ) {
    const char* xPubKeyTest1 = "xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29ESFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8";
    const char* xPrivKeyTest1 = "xprv9s21ZrQH143K3QTDL4LXw2F7HEK3wJUD2nW2nRk4stbPy6cq3jPPqjiChkVvvNKmPGJxWUtg6LnF5kejMRNNU3TGtRBeJgk33yuGBxrMPHi";
    const char* xPubKeyTestAccount1 = "xpub68Gmy5EdvgibQVfPdqkBBCHxA5htiqg55crXYuXoQRKfDBFA1WEjWgP6LHhwBZeNK1VTsfTFUHCdrfp1bgwQ9xv5ski8PX9rL2dZXvgGDnw";
    const char* xPrivKeyTestAccount1 = "xprv9uHRZZhk6KAJC1avXpDAp4MDc3sQKNxDiPvvkX8Br5ngLNv1TxvUxt4cV1rGL5hj6KCesnDYUhd7oWgT11eZG7XnxHrnYeSvkzY7d2bhkJ7";
    const char* xPubKeyTestChain1 = "xpub6ASuArnXKPbfEwhqN6e3mwBcDTgzisQN1wXN9BJcM47sSikHjJf3UFHKkNAWbWMiGj7Wf5uMash7SyYq527Hqck2AxYysAA7xmALppuCkwQ";
    const char* xPrivKeyTestChain1 = "xprv9wTYmMFdV23N2TdNG573QoEsfRrWKQgWeibmLntzniatZvR9BmLnvSxqu53Kw1UmYPxLgboyZQaXwTCg8MSY3H2EU4pWcQDnRnrVA1xe8fs";

    uint8_t testVector1[] = TEST_VECTOR_1;
    
    Key masterKey = Key::MakeMasterKey(testVector1, sizeof(testVector1));

    char descriptor[MAX_DESCRIPTOR_LENGTH] = {'\0'}; 
    masterKey.getDescriptor(descriptor);

    char xPubKey[MAX_DESCRIPTOR_LENGTH] = {'\0'};
    size_t keyLength = 0;
    masterKey.exportXpubKey(xPubKey, keyLength);

    string xPrivKey = masterKey.exportXprivKey();

    BOOST_CHECK(strcmp(xPubKeyTest1, xPubKey) == 0);
    BOOST_CHECK(xPrivKeyTest1 == xPrivKey);

    Key accountKey;
    masterKey.deriveChildKey(MIN_HARDENED_CHILD_INDEX, accountKey);
    memset(xPubKey, '\0', MAX_DESCRIPTOR_LENGTH);

    accountKey.exportXpubKey(xPubKey, keyLength);
    xPrivKey = accountKey.exportXprivKey();

    BOOST_CHECK(strcmp(xPubKeyTestAccount1, xPubKey) == 0);
    BOOST_CHECK(xPrivKeyTestAccount1 == xPrivKey);

    Key chainKey;
    accountKey.deriveChildKey(1, chainKey);
    memset(xPubKey, '\0', MAX_DESCRIPTOR_LENGTH);
    
    chainKey.exportXpubKey(xPubKey, keyLength);
    xPrivKey = chainKey.exportXprivKey();

    BOOST_CHECK(strcmp(xPubKeyTestChain1, xPubKey) == 0);
    BOOST_CHECK(xPrivKeyTestChain1 == xPrivKey);
}

BOOST_AUTO_TEST_CASE( BIP84 ) {
    string xPrivAccount0 = "zprvAdG4iTXWBoARxkkzNpNh8r6Qag3irQB8PzEMkAFeTRXxHpbF9z4QgEvBRmfvqWvGp42t42nvgGpNgYSJA9iefm1yYNZKEm7z6qUWCroSQnE";
    string xPrivReceivingAddress0 = "0330d54fd0dd420a6e5f8d3624f5f3482cae350f79d5f0753bf5beef9c2d91af3c";

    uint8_t masterSeed[] = {
        0x5e, 0xb0, 0x0b, 0xbd, 0xdc, 0xf0, 0x69, 0x08,
        0x48, 0x89, 0xa8, 0xab, 0x91, 0x55, 0x56, 0x81,
        0x65, 0xf5, 0xc4, 0x53, 0xcc, 0xb8, 0x5e, 0x70,
        0x81, 0x1a, 0xae, 0xd6, 0xf6, 0xda, 0x5f, 0xc1,
        0x9a, 0x5a, 0xc4, 0x0b, 0x38, 0x9c, 0xd3, 0x70,
        0xd0, 0x86, 0x20, 0x6d, 0xec, 0x8a, 0xa6, 0xc4,
        0x3d, 0xae, 0xa6, 0x69, 0x0f, 0x20, 0xad, 0x3d,
        0x8d, 0x48, 0xb2, 0xd2, 0xce, 0x9e, 0x38, 0xe4
    };

    Key masterKey = Key::MakeMasterKey(masterSeed, sizeof(masterSeed));

    Key bip84PurposeKey;
    masterKey.deriveChildKey(MIN_HARDENED_CHILD_INDEX + 84, bip84PurposeKey);
    Key coinTypeKey;
    bip84PurposeKey.deriveChildKey(MIN_HARDENED_CHILD_INDEX, coinTypeKey);
    Key accountKey;
    coinTypeKey.deriveChildKey(MIN_HARDENED_CHILD_INDEX, accountKey);
    Key receivingKey;
    accountKey.deriveChildKey(0, receivingKey);
    Key addressKey;
    receivingKey.deriveChildKey(0, addressKey);

    string xPrivKey = accountKey.exportXprivKey();

    BOOST_CHECK(xPrivKey == xPrivAccount0);
}

BOOST_AUTO_TEST_CASE( BIP49 ) {
    string xPrivAccount0 = "yprvAHwhK6RbpuS3dgCYHM5jc2ZvEKd7Bi61u9FVhYMpgMSuZS613T1xxQeKTffhrHY79hZ5PsskBjcc6C2V7DrnsMsNaGDaWev3GLRQRgV7hxF";

    uint8_t masterSeed[] = {
        0x5e, 0xb0, 0x0b, 0xbd, 0xdc, 0xf0, 0x69, 0x08,
        0x48, 0x89, 0xa8, 0xab, 0x91, 0x55, 0x56, 0x81,
        0x65, 0xf5, 0xc4, 0x53, 0xcc, 0xb8, 0x5e, 0x70,
        0x81, 0x1a, 0xae, 0xd6, 0xf6, 0xda, 0x5f, 0xc1,
        0x9a, 0x5a, 0xc4, 0x0b, 0x38, 0x9c, 0xd3, 0x70,
        0xd0, 0x86, 0x20, 0x6d, 0xec, 0x8a, 0xa6, 0xc4,
        0x3d, 0xae, 0xa6, 0x69, 0x0f, 0x20, 0xad, 0x3d,
        0x8d, 0x48, 0xb2, 0xd2, 0xce, 0x9e, 0x38, 0xe4
    };

    Key masterKey = Key::MakeMasterKey(masterSeed, sizeof(masterSeed));
    Key bip49PurposeKey;
    masterKey.deriveChildKey(MIN_HARDENED_CHILD_INDEX + 49, bip49PurposeKey);
    Key coinTypeKey;
    bip49PurposeKey.deriveChildKey(MIN_HARDENED_CHILD_INDEX, coinTypeKey);
    Key accountKey;
    coinTypeKey.deriveChildKey(MIN_HARDENED_CHILD_INDEX, accountKey);
    Key receivingKey;
    accountKey.deriveChildKey(0, receivingKey);
    Key addressKey;
    receivingKey.deriveChildKey(0, addressKey);

    string privKey = accountKey.exportXprivKey();

    BOOST_CHECK(privKey == xPrivAccount0);
}

BOOST_AUTO_TEST_CASE( calculateAddressFromSeed ) {
    uint8_t masterSeed[] = {
        0x5e, 0xb0, 0x0b, 0xbd, 0xdc, 0xf0, 0x69, 0x08,
        0x48, 0x89, 0xa8, 0xab, 0x91, 0x55, 0x56, 0x81,
        0x65, 0xf5, 0xc4, 0x53, 0xcc, 0xb8, 0x5e, 0x70,
        0x81, 0x1a, 0xae, 0xd6, 0xf6, 0xda, 0x5f, 0xc1,
        0x9a, 0x5a, 0xc4, 0x0b, 0x38, 0x9c, 0xd3, 0x70,
        0xd0, 0x86, 0x20, 0x6d, 0xec, 0x8a, 0xa6, 0xc4,
        0x3d, 0xae, 0xa6, 0x69, 0x0f, 0x20, 0xad, 0x3d,
        0x8d, 0x48, 0xb2, 0xd2, 0xce, 0x9e, 0x38, 0xe4
    };

    Key masterKey = Key::MakeMasterKey(masterSeed, sizeof(masterSeed));

    char xPubKey[MAX_DESCRIPTOR_LENGTH] = {'\0'};

    string xPrivKey = masterKey.exportXprivKey();
    char descriptor[MAX_DESCRIPTOR_LENGTH] = {'\0'};

    Key purposeKey;
    masterKey.deriveChildKey(MIN_HARDENED_CHILD_INDEX + 44, purposeKey);
    Key coinTypeKey;
    purposeKey.deriveChildKey(MIN_HARDENED_CHILD_INDEX, coinTypeKey);
    Key accountKey;
    coinTypeKey.deriveChildKey(MIN_HARDENED_CHILD_INDEX, accountKey);
    Key receivingKey;
    accountKey.deriveChildKey(0, receivingKey);
    Key addressKey;
    receivingKey.deriveChildKey(0, addressKey);

    xPrivKey = accountKey.exportXprivKey();

    xPrivKey = addressKey.exportXprivKey();
    size_t keyLength = 0;
    addressKey.exportXpubKey(xPubKey, keyLength);
    receivingKey.exportDescriptor(descriptor, RECEIVING);
}

BOOST_AUTO_TEST_SUITE_END()
