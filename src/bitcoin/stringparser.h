#ifndef PARSER
#define PARSER

#include <iostream>
#include <string>
#include <stdint.h>

using namespace std;

bool ParseFixedPoint(std::string_view val, int decimals, int64_t *amount_out);

/**
 * Tests if the given character is a decimal digit.
 * @param[in] c     character to test
 * @return          true if the argument is a decimal digit; otherwise false.
 */
constexpr bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

std::string escapeQuotes(const std::string &input);

#endif