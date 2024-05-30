#ifndef BYTEUTILS
#define BYTEUTILS

#include <stdint.h>
#include <stdio.h>


void uint32ToBytes(uint32_t number, uint8_t* bytes);


void addAndModulus256bitIntegers(const uint8_t* num1, const uint8_t* num2, uint8_t* result);


void byteToHex(uint8_t byte, char* hex);


void uint32ToDecimalString(uint32_t value, char* buffer, size_t& len);


void calculateKeyFingerprint(const uint8_t* key, uint8_t* fingerprint);

// Generates a destination address for a P2WPKH script
// https://learnmeabitcoin.com/technical/script/p2wpkh/#address
void generateP2WPKHAddress(const uint8_t* compressedPublicKey, char* address);


void exportWIFKey(const uint8_t* data, const uint8_t dataLen, char* wifKey);


void print_string(uint8_t* data, uint8_t size);


#endif
