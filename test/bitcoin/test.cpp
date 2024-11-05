#include "../../src/bitcoin/amount.h"
#include "../../src/bitcoin/stringparser.h"
#include "../../src/bitcoin/stringparser.cpp"
#include "../../src/bitcoin/nodemanager.cpp"
#include "../../src/bitcoin/nodemanager.h"
#include "../../src/bitcoin/jsonparser.h"
#include "../../src/bitcoin/jsonparser.cpp"

#define BOOST_TEST_MODULE Main
#define PARSER_TESTS parser_tests

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( Json)

BOOST_AUTO_TEST_CASE( testParseDecimalValue )
{
    CAmount amount;
    string stringValue = "0.11476785";
    bool parseOk = ParseFixedPoint(stringValue, 8, &amount);
    BOOST_CHECK(parseOk == true);
    BOOST_CHECK(11476785 == amount);
}

BOOST_AUTO_TEST_CASE( testParseIntValue )
{
    CAmount amount;
    string stringValue = "2";
    bool parseOk = ParseFixedPoint(stringValue, 8, &amount);
    BOOST_CHECK(parseOk == true);
    BOOST_CHECK(200000000 == amount);
}

BOOST_AUTO_TEST_CASE( testParseIntValueWithDecimals )
{
    CAmount amount;
    string stringValue = "2.0";
    bool parseOk = ParseFixedPoint(stringValue, 8, &amount);
    BOOST_CHECK(parseOk == true);
    BOOST_CHECK(200000000 == amount);
}

BOOST_AUTO_TEST_CASE( testBTCtoSatsConversion )
{
    CAmount amount = 4500000;
    double sats = (double)amount/COIN;
    cout << "sats is > " << sats;
    BOOST_CHECK(sats == 0.045);
}

BOOST_AUTO_TEST_CASE( testListUnspentApi )
{
    NodeManager node = NodeManager();
    node.runApi(LIST_UNSPENT, "[]", string("wallet/watchonly"), false);
    cout << "Response is > " << node.apiResponse << endl << endl;
    JsonObject jsonResponse = JsonObject(node.apiResponse);
    string result = jsonResponse.getChildAsString("result");
    cout << "Result is > " << result;
    cout << "Response is > " << JsonObject(result).length();
    BOOST_CHECK(JsonObject(result).length() == 6);
}

BOOST_AUTO_TEST_SUITE_END()
