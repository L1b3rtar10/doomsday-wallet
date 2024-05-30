#include "base58.h"

#include <iostream>
#include <unistd.h>
#include <cstring>


void EncodeBase58(uint8_t* input, uint8_t size, char* key)
{
    static const char pszBase58[] = {'1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','J','K','L','M','N','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','m','n','o','p','q','r','s','t','u','v','w','x','y','z'};
    
    // Skip & count leading zeroes.
    int zeroes = 0;
    int length = 0;
    uint8_t index = 0;
    while (index < size && input[index] == 0) {
        zeroes++;
        index++;
    }

    // Allocate enough space in big-endian base58 representation.
    uint8_t newsize = size * 138 / 100 + 1; // log(256) / log(58), rounded up.
    uint8_t base58vector[newsize];
    memset(base58vector, 0, newsize * sizeof(uint8_t));

    // Process the bytes.

    while (index < size) {
        unsigned int carry = input[index];
        int i = 0;

        // Apply "b58 = b58 * 256 + ch".
        for (int8_t pos = newsize - 1; (carry != 0 || i < length) && pos >= 0; pos--, i++) {
            carry += 256 * base58vector[pos];
            base58vector[pos] = carry % 58;
            carry /= 58;
        }

        if (carry != 0) {
            printf("Error! Carry is not 0! > %d", carry);
            exit(1);
        }
        
        length = i;
        index++;
    }

    for (int i = 0; i < length; i++) {
        key[i] = pszBase58[base58vector[newsize - length + i]];
    }
}
