#include "byteutils.h"
#include "ripemd160.h"
#include "sha-256.h"
#include "bech32.h"
#include "byteutils.h"
#include "../crypto/base58.h"


void uint32ToBytes(uint32_t number, uint8_t* bytes) {
    bytes[0] = (number >> 24) & 0xFF; // Extract the most significant byte
    bytes[1] = (number >> 16) & 0xFF; // Extract the second most significant byte
    bytes[2] = (number >> 8) & 0xFF;  // Extract the second least significant byte
    bytes[3] = number & 0xFF;         // Extract the least significant byte
}


void addAndModulus256bitIntegers(const uint8_t* num1, const uint8_t* num2, uint8_t* result) {
    uint16_t sum[32]; // Use a larger container to store the sum
    uint16_t carry = 0;
    for (int i = 31; i >= 0; --i) {
        sum[i] = num1[i] + num2[i] + carry;
        carry = sum[i] >> 8; // Carry over to the next byte
    }

    // Modulus order: n = 115792089237316195423570985008687907852837564279074904382605163141518161494337
    // https://en.bitcoin.it/wiki/Secp256k1
    // Apply modulus to bring the sum back into the 256-bit range
    const uint8_t modulus[32] = {
        0x79, 0xBE, 0x66, 0x7E, 0xF9, 0xDC, 0xBB, 0xAC,
        0xC5, 0x76, 0x24, 0x36, 0x71, 0x8E, 0x9F, 0xA5,
        0x33, 0xC4, 0xD7, 0x44, 0x79, 0x77, 0x60, 0x91,
        0x63, 0x8F, 0x4C, 0x70, 0x21, 0x01, 0x00, 0x00
    };
    carry = 0;
    for (int i = 31; i >= 0; --i) {
        uint16_t sumWithCarry = sum[i] + carry;
        if (sumWithCarry >= modulus[i]) {
            result[i] = sumWithCarry - modulus[i];
            carry = 1; // Set carry to 1 if there's a borrow
        } else {
            result[i] = sumWithCarry;
            carry = 0; // Set carry to 0 otherwise
        }
    }
}

void byteToHex(uint8_t byte, char* hex) {
    constexpr char hexChars[] = "0123456789abcdef";
    hex[0] = hexChars[(byte >> 4) & 0x0F];
    hex[1] = hexChars[byte & 0x0F];
}

void uint32ToDecimalString(uint32_t value, char* buffer, size_t& len) {
    // Handle the case of 0 separately
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        len = 2;
        return;
    }

    // Start building the decimal string from the end
    int index = 0;
    while (value > 0) {
        buffer[index++] = '0' + (value % 10); // Convert digit to character
        value /= 10;
    }

    // Reverse the string
    int start = 0;
    int end = index - 1;
    while (start < end) {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        ++start;
        --end;
    }

    // Null-terminate the string
    buffer[index] = '\0';
    len = index + 1;
}

void calculateKeyFingerprint(const uint8_t* key, uint8_t* fingerprint) {
    unsigned char sha256Hash[SIZE_OF_SHA_256_HASH] = {0x00};

    calc_sha_256(sha256Hash, key, 33);
    CRIPEMD160{}.Write(sha256Hash, SIZE_OF_SHA_256_HASH).Finalize(fingerprint);
}

void generateP2WPKHAddress(const uint8_t* key, char* address) {
    const char HRP[] = "bc";
    uint8_t data[20] = {0x00};
    
    unsigned char sha256Hash[SIZE_OF_SHA_256_HASH] = {0x00};

    calc_sha_256(sha256Hash, key, 33);
    CRIPEMD160{}.Write(sha256Hash, SIZE_OF_SHA_256_HASH).Finalize(data);

    segwit_addr_encode(address, HRP, 0, data, sizeof(data));
}

void exportWIFKey(const uint8_t* data, const uint8_t dataLen, char* wifKey) {
    uint8_t* numericInput = new uint8_t[dataLen + 1];
    numericInput[0] = {0x80};

    
    // Add 0x80 prefix
    memcpy(&numericInput[1], data, dataLen);

    uint8_t hash[SIZE_OF_SHA_256_HASH];

    // Calc SHA256
    calc_sha_256(hash, numericInput, dataLen + 1);

    // Calc SHA256
    uint8_t secondHash[SIZE_OF_SHA_256_HASH];
    calc_sha_256(secondHash, &hash, SIZE_OF_SHA_256_HASH);

    // Concatenate checksum to prefixed input 
    uint8_t extendedInput[dataLen + 1 + 4];
    memcpy(extendedInput, numericInput, dataLen + 1);

    // Append checksum
    memcpy(&extendedInput[dataLen + 1], secondHash, 4);

    EncodeBase58(extendedInput, dataLen + 1 + 4, wifKey);

    memset(numericInput, 0x00, dataLen + 1);
    delete[] numericInput;
}

void print_string(uint8_t* data, uint8_t size) {
	printf("\n");
    size_t i;
	for (i = 0; i < size; i++) {
        printf("%02X", data[i]);
	}
    printf("\n");
}
