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
/*
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


BOOST_AUTO_TEST_CASE( testCreateWallet )
{
    NodeManager nodeManager = NodeManager();
    JsonObject parameters;
    string name("test2");
    parameters.addKVString("wallet_name", name);
    parameters.addKVBool("disable_private_keys", true);
    parameters.addKVBool("blank", true);
    parameters.addKVBool("avoid_reuse", true);
    parameters.addKVBool("descriptors", true);
    parameters.addKVBool("load_on_startup", true);

    nodeManager.runApi(CREATE_WALLET, parameters.toJson(), "", false);
    BOOST_CHECK(JsonObject(nodeManager.apiResponse).getChildAsJsonObject("result").getChildAsString("name") == name);
}


BOOST_AUTO_TEST_CASE( getDescriptorInfo )
{
    NodeManager nodeManager = NodeManager();
    string descriptor("pkh([de46b9fe/44h/0h/0h]xpub6EynTVQkEHEbeo8PkuQyNC85ZJLK2xMNnzMf3ukhMuBznaWxn8mhWD98Ann7Z17tXhLYNU25otLq7DMqHGZa6vXVqznb9PvaA3TabCVVBn7/0/*)");

    nodeManager.runApi(GET_DESCRIPTOR_INFO, "[\"" + descriptor + "\"]", "", false);
    BOOST_CHECK(JsonObject(nodeManager.apiResponse).getChildAsJsonObject("result").getChildAsString("checksum") == "5n89newv");
}*/

BOOST_AUTO_TEST_CASE( importDescriptor )
{
    NodeManager nodeManager = NodeManager();
    string descriptor("pkh([de46b9fe/44h/0h/0h]xpub6EynTVQkEHEbeo8PkuQyNC85ZJLK2xMNnzMf3ukhMuBznaWxn8mhWD98Ann7Z17tXhLYNU25otLq7DMqHGZa6vXVqznb9PvaA3TabCVVBn7/0/*)");
    nodeManager.runApi(GET_DESCRIPTOR_INFO, "[\"" + descriptor + "\"]", "", false);

    JsonObject descriptorData;
    string descriptorJson = JsonObject(nodeManager.apiResponse).getChildAsJsonObject("result").getChildAsString("descriptor");
    descriptorData.addKVString("desc", descriptorJson);
    // Get the current time as a time_point
    auto now = std::chrono::system_clock::now();

    // Convert to milliseconds since epoch
    uint64_t nowMillis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    descriptorData.addKVInt("timestamp", nowMillis);
    //descriptorData.addKVInt("range", now);
    nodeManager.runApi(IMPORT_DESCRIPTORS, "[[" + descriptorData.toJson() + "]]", "wallet/rr", false);
    cout << endl << nodeManager.apiResponse << endl;

    BOOST_CHECK(true == true);
}

BOOST_AUTO_TEST_SUITE_END()
