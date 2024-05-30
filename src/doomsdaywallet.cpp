#include <iostream>
#include "crypto/sha-256.h"
#include "crypto/key.h"
#include <stdlib.h>

#define EXIT_CODE '9'

#define OPTIONS_MENU "\n\n+++ Choose an option +++\n\
----- 1. Manage wallets\n\
----- 2. Create safe wallet\n\
----- 3. Send amount to address\n\
----- 4. Decode transaction\n\
----- 5. Select default tx fee\n\
----- 6. Generate new address\n"

void printMenu()
{
    std::cout << OPTIONS_MENU << "----- " << EXIT_CODE << ". Exit\n\n+++ Selection: ";
    return;
}

int main(int argc, char **argv) {

    std::cout << "Welcome to Doomsday Wallet";

    return 0;
}
