#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <openssl/evp.h>

#include "random_generator.h"

#define RANDOM_ENTROPY_SIZE 1048576
#define ENTROPY_SIZE 64
#define PBKDF2_ITERATIONS 250000


string generateRandomSeed() {
    // Array to hold the random bytes
    char randomBytes[RANDOM_ENTROPY_SIZE] = {'\0'};
    unsigned char nonce[RANDOM_ENTROPY_SIZE] = {'\0'};

    unsigned char seed[ENTROPY_SIZE];

    // Open /dev/urandom to read random data
    ifstream urandom("/dev/urandom", ios::in | ios::binary);

    if (!urandom) {
        cerr << "Failed to open /dev/urandom" << endl;
        return string("Error!");
    }

    // Read 64 random bytes from /dev/urandom
    urandom.read(reinterpret_cast<char*>(randomBytes), RANDOM_ENTROPY_SIZE);
    urandom.read(reinterpret_cast<char*>(nonce), RANDOM_ENTROPY_SIZE);
    urandom.close();

    if (!PKCS5_PBKDF2_HMAC(randomBytes, RANDOM_ENTROPY_SIZE, nonce, RANDOM_ENTROPY_SIZE, PBKDF2_ITERATIONS, \
        EVP_sha512(), ENTROPY_SIZE, seed)) {
        std::cerr << "Error: PBKDF2 derivation failed\n";
        exit(EXIT_FAILURE);
    }

    // Create a stringstream to hold the formatted output
    stringstream output;
    // Start the output in the required format
    output << "#define RANDOM_SEED { \\" << endl;

    // Print the bytes in hexadecimal format with 8 bytes per line
    for (int i = 0; i < ENTROPY_SIZE; ++i) {
        // Print each byte as hex
        output << " 0x" << hex << setw(2) << setfill('0')
                  << static_cast<int>(seed[i]);

        // Add a comma unless it's the last byte
        if (i != ENTROPY_SIZE - 1) {
            output << ",";
        }

        // Print a new line after every 8 bytes
        if ((i + 1) % 8 == 0) {
            output << " \\" << endl;
        } else {
            output << " ";
        }
    }

    // Close the output block
    output << "}" << endl;

    return output.str();
}
