#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

#include "../../src/bitcoin/jsonparser.h"
#include "../../src/bitcoin/jsonparser.cpp" // Must include all files, .cpp included!
#include "../../src/bitcoin/stringparser.h"
#include "../../src/bitcoin/stringparser.cpp"

#define BOOST_TEST_MODULE Main
#define JSON_TESTS json_tests

#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(Json)

BOOST_AUTO_TEST_CASE(findKeys)
{
    JsonObject jsonObject(
    "{\
        \"mine\": { \n\
            \"trusted\": 0.00000000,\n\
            \"untrusted_pending\": 0.00000000,\n\
            \"immature\": 0.00000000,\n\
            \"used\": 0.00000000\n\
        },\n\
        \"watchonly\": {\n\
            \"trusted\": 0.11476785,\n\
            \"untrusted_pending\": 0.00000000,\n\
            \"immature\": 0.00000000\n\
        }\n\
    }");
    BOOST_CHECK(jsonObject.hasKey("mine") == true);
    BOOST_CHECK(jsonObject.hasKey("watchonly") == true);
}

BOOST_AUTO_TEST_CASE( getChild)
{
    JsonObject jsonObject(
    "{\
        \"mine\": { \n\
            \"trusted\": 0.00000000,\n\
            \"untrusted_pending\": 0.00000000,\n\
            \"immature\": 0.00000000,\n\
            \"used\": 0.00000000\n\
        },\n\
        \"watchonly\": {\n\
            \"trusted\": 0.11476785,\n\
            \"untrusted_pending\": 0.00000000,\n\
            \"immature\": 0.00000000\n\
        }\n\
    }");

    JsonObject mineWalletJson = jsonObject.getChildAsJsonObject("mine");
    BOOST_CHECK(mineWalletJson.hasKey("trusted") == true);
    BOOST_CHECK(mineWalletJson.getChildAsString("trusted") == "0.00000000");
    BOOST_CHECK(mineWalletJson.getChildAsSatsAmount("trusted") == 0);

    JsonObject watchOnly = jsonObject.getChildAsJsonObject("watchonly");
    BOOST_CHECK(watchOnly.getChildAsString("trusted") == "0.11476785");
    BOOST_CHECK(watchOnly.getChildAsSatsAmount("trusted") == 11476785);
}

BOOST_AUTO_TEST_CASE(testAmounts)
{
    JsonObject jsonObject(
    "{\
        \"mine\": { \n\
            \"trusted\": 0.00000000,\n\
            \"untrusted_pending\": 0.00000000,\n\
            \"immature\": 0.00000000,\n\
            \"used\": 0.00000000\n\
        },\n\
        \"watchonly\": {\n\
            \"trusted\": 0.11476785,\n\
            \"untrusted_pending\": 0.00000000,\n\
            \"immature\": 0.00200000\n\
        }\n\
    }");

    JsonObject watchOnly = jsonObject.getChildAsJsonObject("watchonly");
    BOOST_CHECK(watchOnly.getChildAsString("trusted") == "0.11476785");
    BOOST_CHECK(watchOnly.getChildAsSatsAmount("trusted") == 11476785);
    BOOST_CHECK(watchOnly.getChildAsSatsAmount("immature") == 200000);
}


BOOST_AUTO_TEST_CASE(arrayOfObjects)
{
    JsonObject jsonObject("[{\"txid\":\"aa1cbc766299cb1d697b8316dec7054c6f94ac99e71a72a54e6241321b67c833\",\"vout\":1,\"address\":\"bc1qy3h8phrfchasu3yzmsu3wq2algqqc3t8m0w9gv\",\"label\":\"safe\",\"scriptPubKey\":\"0014246e70dc69c5fb0e4482dc3917015dfa000c4567\",\"amount\":0.03308520,\"confirmations\":3510,\"spendable\":false,\"solvable\":true,\"desc\":\"wpkh([0d55dd29/0'/0'/3']020f7b8b22c3bb6fbab3e248e39a221208c3459ba20c8047e11a52d65e7e826d79)#q3j4edy6\",\"reused\":false,\"safe\":true},{\"txid\":\"0776bd53a390b365d2ff62edf76ebafe7a198ec97d9b6712e0e8311a35d53090\",\"vout\":0,\"address\":\"bc1q7wv8pdd75rv3qd2zptsyxupx5xyrfn6tnz4qwq\",\"label\":\"safe\",\"scriptPubKey\":\"0014f39870b5bea0d91035420ae0437026a18834cf4b\",\"amount\":0.03500000,\"confirmations\":28476,\"spendable\":false,\"solvable\":true,\"desc\":\"wpkh([f39870b5]0226c7c6d768d46510e2f43c0d7afdfcbb1f85b7a11319b51b780af95cc8195793)#k5cgrwsx\",\"reused\":false,\"safe\":true},{\"txid\":\"3d6f0ccba9bca74ce5fac2026773c86fceb3372343c38a3430edb2388e54faaf\",\"vout\":16,\"address\":\"bc1q4xnv2hftsz82qtl4nzsjf3szv7yv3smy3nwme2\",\"label\":\"safe\",\"scriptPubKey\":\"0014a9a6c55d2b808ea02ff598a124c6026788c8c364\",\"amount\":0.04668265,\"confirmations\":24065,\"spendable\":false,\"solvable\":true,\"desc\":\"wpkh([0d55dd29/0'/0'/2']02bb5a0259c12bc4fb8c7ee505044e367e6c5715eac0ede2b0cb3d2514958a6a70)#t26gk727\",\"reused\":false,\"safe\":true}]");
    BOOST_CHECK(jsonObject.isArray() == 1);
    JsonObject firstTxJson = jsonObject.getChildAt(0);
    BOOST_CHECK(firstTxJson.getChildAsString("txid") == "aa1cbc766299cb1d697b8316dec7054c6f94ac99e71a72a54e6241321b67c833");
    BOOST_CHECK(firstTxJson.getChildAsInt("vout") == 1);
    BOOST_CHECK(firstTxJson.getChildAsInt("confirmations") == 3510);

    JsonObject secondTxJson = jsonObject.getChildAt(1);
    BOOST_CHECK(secondTxJson.getChildAsString("txid") == "0776bd53a390b365d2ff62edf76ebafe7a198ec97d9b6712e0e8311a35d53090");
    BOOST_CHECK(secondTxJson.getChildAsInt("vout") == 0);
}

BOOST_AUTO_TEST_CASE(createJsonObject)
{
    JsonObject jsonObject;
    jsonObject.addKVString("key1", "value1");
    jsonObject.addKVString("key2", "value2");
    jsonObject.addKVAmount("keyAmount", 1234556);
    jsonObject.addKVInt("intValue", 12);
    jsonObject.addKVBool("boolValue", true);

    BOOST_CHECK(jsonObject.toJson() == "{\"key1\":\"value1\",\"key2\":\"value2\",\"keyAmount\":0.012346,\"intValue\":12,\"boolValue\":true}");
}

BOOST_AUTO_TEST_CASE(parseBoolean)
{
  JsonObject jsonObject("{\"hex\": \"0200000000010333c8671b3241624ea5721ae799ac946f4c05c7de16837b691dcb996276bc1caa0100000000feffffff9030d5351a31e8e012679b7dc98e197afeba6ef7ed62ffd265b390a353bd76070000000000feffffffaffa548e38b2ed30348ac3432337b3ce6fc8736702c2fae54ca7bca9cb0c6f3d1000000000feffffff0200127a0000000000160014a9a6c55d2b808ea02ff598a124c6026788c8c3649e01350000000000160014cf5c9e6b7dd0684757d0c9da5de21d16a6aaddff024730440220430a1519346521409181fb813508e0ac1cd52bea2cec181cdc4cdc7d81a1ee69022071cd346c6361bfae53bb662563134ef70c7fb1e82e91fb50b054fbe695893df40121020f7b8b22c3bb6fbab3e248e39a221208c3459ba20c8047e11a52d65e7e826d790247304402205d59af7299e2df7842d531db68578321ee2333605cd4416763e9b91392f8cf490220406d2c96aa7e8ddad3927ee6ff942ae8a6e28ab5d336a9a9e3f9817186ce5cf101210226c7c6d768d46510e2f43c0d7afdfcbb1f85b7a11319b51b780af95cc81957930247304402204bfb1f590af9203a4dbcdb11394499a7fadd151b1f34e0d249161640fb4e7bdc022021ead7743feae6ce76fc5973b27be44ad4c3a09bedbb462273e006985056bd3d012102bb5a0259c12bc4fb8c7ee505044e367e6c5715eac0ede2b0cb3d2514958a6a7000000000\",\"complete\": true }");
  BOOST_CHECK(jsonObject.getChildAsString("complete") == "true");
}

BOOST_AUTO_TEST_CASE(findUsedAddress)
{
    JsonObject transactions(
    "[{\
         \"involvesWatchonly\": true,\n\
        \"address\": \"bc1qy62zs5ftw75z9j9rgj94auaq4cz597ta44cqyd\",\n\
        \"category\": \"receive\",\n\
        \"amount\": 0.00384185,\n\
        \"label\": \"old\",\n\
        \"vout\": 1,\n\
        \"confirmations\": 31398,\n\
        \"blockhash\": \"00000000000000000001a0098f7952716d288cd2268394702700a7f6d43eccd5\",\n\
        \"blockheight\": 737982,\n\
        \"blockindex\": 3089,\n\
        \"blocktime\": 1653565889,\n\
        \"txid\": \"0776bd53a390b365d2ff62edf76ebafe7a198ec97d9b6712e0e8311a35d53090\",\n\
        \"walletconflicts\": [\n\
        ],\n\
        \"time\": 1653564950,\n\
        \"timereceived\": 1653564950,\n\
        \"bip125-replaceable\": \"no\"\n\
    },\n\
    {\n\
        \"involvesWatchonly\": true,\n\
        \"address\": \"bc1q7wv8pdd75rv3qd2zptsyxupx5xyrfn6tnz4qwq\",\n\
        \"category\": \"receive\",\n\
        \"amount\": 0.03500000,\n\
        \"label\": \"safe\",\n\
        \"vout\": 0,\n\
        \"confirmations\": 31398,\n\
        \"blockhash\": \"00000000000000000001a0098f7952716d288cd2268394702700a7f6d43eccd5\",\n\
        \"blockheight\": 737982,\n\
        \"blockindex\": 3089,\n\
        \"blocktime\": 1653565889,\n\
        \"txid\": \"0776bd53a390b365d2ff62edf76ebafe7a198ec97d9b6712e0e8311a35d53090\",\n\
        \"walletconflicts\": [\n\
        ],\n\
        \"time\": 1653564950,\n\
        \"timereceived\": 1653564950,\n\
        \"bip125-replaceable\": \"no\"\n\
    },\n\
    {\n\
        \"involvesWatchonly\": true,\n\
        \"address\": \"bc1qy62zs5ftw75z9j9rgj94auaq4cz597ta44cqyd\",\n\
        \"category\": \"send\",\n\
        \"amount\": -0.00384185,\n\
        \"label\": \"old\",\n\
        \"vout\": 1,\n\
        \"fee\": -0.00002090,\n\
        \"confirmations\": 31398,\n\
        \"blockhash\": \"00000000000000000001a0098f7952716d288cd2268394702700a7f6d43eccd5\",\n\
        \"blockheight\": 737982,\n\
        \"blockindex\": 3089,\n\
        \"blocktime\": 1653565889,\n\
        \"txid\": \"0776bd53a390b365d2ff62edf76ebafe7a198ec97d9b6712e0e8311a35d53090\",\n\
        \"walletconflicts\": [\n\
        ],\n\
        \"time\": 1653564950,\n\
        \"timereceived\": 1653564950,\n\
        \"bip125-replaceable\": \"no\",\n\
        \"abandoned\": false\n\
    },\n\
    {\n\
        \"involvesWatchonly\": true,\n\
        \"address\": \"bc1q7wv8pdd75rv3qd2zptsyxupx5xyrfn6tnz4qwq\",\n\
        \"category\": \"send\",\n\
        \"amount\": -0.03500000,\n\
        \"label\": \"safe\",\n\
        \"vout\": 0,\n\
        \"fee\": -0.00002090,\n\
        \"confirmations\": 31398,\n\
        \"blockhash\": \"00000000000000000001a0098f7952716d288cd2268394702700a7f6d43eccd5\",\n\
        \"blockheight\": 737982,\n\
        \"blockindex\": 3089,\n\
        \"blocktime\": 1653565889,\n\
        \"txid\": \"0776bd53a390b365d2ff62edf76ebafe7a198ec97d9b6712e0e8311a35d53090\",\n\
        \"walletconflicts\": [\n\
        ],\n\
        \"time\": 1653564950,\n\
        \"timereceived\": 1653564950,\n\
        \"bip125-replaceable\": \"no\",\n\
        \"abandoned\": false\n\
    }]");

    string findAddress = "bc1q7wv8pdd75rv3qd2zptsyxupx5xyrfn6tnz4qwq";
    bool found = false;
    for (size_t i = 0; i < transactions.length(); i++) {
        JsonObject transaction = JsonObject(transactions.getChildAt(i));
        string address = transaction.getChildAsString("address");
        address.erase(
        std::remove_if(
            address.begin(), address.end(), [](char const c) {
                return '"' == c;
            }),
            address.end()
        );
        if (address == findAddress) {
            found = true;
        }
    }

    BOOST_CHECK(found == true);

    findAddress = "bc1q7wv8pdd75rv3qd2zptsyxupx5xyrfn6tnz4qwz";
    found = false;
    for (size_t i = 0; i < transactions.length(); i++) {
        JsonObject transaction = JsonObject(transactions.getChildAt(i));
        string address = transaction.getChildAsString("address");
        address.erase(
        std::remove_if(
            address.begin(), address.end(), [](char const c) {
                return '"' == c;
            }),
            address.end()
        );
        if (address == findAddress) {
            found = true;
        }
    }

    BOOST_CHECK(found == false);
}

/*
BOOST_AUTO_TEST_CASE(testDescriptorInputFormat)
{
    string descriptor = "wpkh([0d55dd29/0'/0'/3']020f7b8b22c3bb6fbab3e248e39a221208c3459ba20c8047e11a52d65e7e826d79)#q3j4edy6";
    string import = createDescriptorImport(descriptor, true, "");
    string test("[{\"desc\":\"wpkh([0d55dd29/0'\"'\"'/0'\"'\"'/3'\"'\"']020f7b8b22c3bb6fbab3e248e39a221208c3459ba20c8047e11a52d65e7e826d79)#q3j4edy6\",\"timestamp\":\"now\",\"internal\":true}]"); 
    BOOST_CHECK(import == test);
}
*/


BOOST_AUTO_TEST_CASE(testParseTransactions) {
    string apiResponse("{\"result\":[{\"txid\":\"aa1cbc766299cb1d697b8316dec7054c6f94ac99e71a72a54e6241321b67c833\",\"vout\":1,\"address\":\"bc1qy3h8phrfchasu3yzmsu3wq2algqqc3t8m0w9gv\",\"label\":\"legacy\",\"scriptPubKey\":\"0014246e70dc69c5fb0e4482dc3917015dfa000c4567\",\"amount\":0.03308520,\"confirmations\":104656,\"spendable\":false,\"solvable\":false,\"reused\":false,\"safe\":true},{\"txid\":\"0a784c51b36fefb177376848295a1cf3e982f0d59b25439dab73ba8b72ce9b58\",\"vout\":22,\"address\":\"bc1q3vftufazx03ucp6sdwq9vq6hkkng036k34my24\",\"label\":\"legacy\",\"scriptPubKey\":\"00148b12be27a233e3cc07506b80560357b5a687c756\",\"amount\":0.05844221,\"confirmations\":94383,\"spendable\":false,\"solvable\":false,\"reused\":false,\"safe\":true}],\"error\":null,\"id\":\"curltest\"}");
    JsonObject responseJson(apiResponse);
    JsonObject transactions = JsonObject(responseJson.getChildAsJsonObject("result"));

    BOOST_CHECK(responseJson.isArray() == false);
    BOOST_CHECK(transactions.isArray() == true);

    JsonObject transaction = JsonObject(transactions.getChildAt(0));

    BOOST_CHECK(transaction.hasKey("txid") == true);
    BOOST_CHECK(transaction.getChildAsString("txid") == string("aa1cbc766299cb1d697b8316dec7054c6f94ac99e71a72a54e6241321b67c833"));
    BOOST_CHECK(transaction.getChildAsInt("vout") == 1);
    BOOST_CHECK(transaction.getChildAsSatsAmount("amount") == 3308520);
    BOOST_CHECK(transaction.getChildAsString("label") == "legacy");
    BOOST_CHECK(transaction.getChildAsBool("spendable") == false);

    JsonObject transaction2 = JsonObject(transactions.getChildAt(1));
    BOOST_CHECK(transaction2.getChildAsBool("safe") == true);
}

BOOST_AUTO_TEST_CASE(createJson) {
    CAmount amountSats = 500000;
    JsonObject jsonObject = JsonObject();
    jsonObject.addKVAmount("bc1anaddress", amountSats);
    JsonObject moreInfo = JsonObject();
    moreInfo.addKVString("test", "testvalue");
    jsonObject.addJsonObject(moreInfo);
    BOOST_CHECK(jsonObject.toJson() == "{\"bc1anaddress\":0.005000,{\"test\":\"testvalue\"}}");
}

BOOST_AUTO_TEST_CASE(parseJson) {
    string response("[{\"txid\":\"aa1cbc766299cb1d697b8316dec7054c6f94ac99e71a72a54e6241321b67c833\",\"vout\":1,\"address\":\"bc1qy3h8phrfchasu3yzmsu3wq2algqqc3t8m0w9gv\",\"label\":\"legacy\",\"scriptPubKey\":\"0014246e70dc69c5fb0e4482dc3917015dfa000c4567\",\"amount\":0.03308520,\"confirmations\":105354,\"spendable\":false,\"solvable\":false,\"reused\":false,\"safe\":true},{\"txid\":\"0a784c51b36fefb177376848295a1cf3e982f0d59b25439dab73ba8b72ce9b58\",\"vout\":22,\"address\":\"bc1q3vftufazx03ucp6sdwq9vq6hkkng036k34my24\",\"label\":\"legacy\",\"scriptPubKey\":\"00148b12be27a233e3cc07506b80560357b5a687c756\",\"amount\":0.05844221,\"confirmations\":95081,\"spendable\":false,\"solvable\":false,\"reused\":false,\"safe\":true},{\"txid\":\"9ef1a4475188ab870ab6d768a9e7fb83cb2022b9a89ae296f8f998323fc4a06c\",\"vout\":8,\"address\":\"bc1q7aces9jfy6ykqawwn03yc9p24xt92gvctxvyf3\",\"label\":\"legacy\",\"scriptPubKey\":\"0014f77198164926896075ce9be24c142aa996552198\",\"amount\":0.04807444,\"confirmations\":73179,\"spendable\":false,\"solvable\":false,\"reused\":false,\"safe\":true},{\"txid\":\"0776bd53a390b365d2ff62edf76ebafe7a198ec97d9b6712e0e8311a35d53090\",\"vout\":0,\"address\":\"bc1q7wv8pdd75rv3qd2zptsyxupx5xyrfn6tnz4qwq\",\"label\":\"legacy\",\"scriptPubKey\":\"0014f39870b5bea0d91035420ae0437026a18834cf4b\",\"amount\":0.03500000,\"confirmations\":130320,\"spendable\":false,\"solvable\":false,\"reused\":false,\"safe\":true},{\"txid\":\"3d6f0ccba9bca74ce5fac2026773c86fceb3372343c38a3430edb2388e54faaf\",\"vout\":16,\"address\":\"bc1q4xnv2hftsz82qtl4nzsjf3szv7yv3smy3nwme2\",\"label\":\"legacy\",\"scriptPubKey\":\"0014a9a6c55d2b808ea02ff598a124c6026788c8c364\",\"amount\":0.04668265,\"confirmations\":125909,\"spendable\":false,\"solvable\":false,\"reused\":false,\"safe\":true},{\"txid\":\"a68d2b515435e4ead5ac26840027f79ab565f024138921b937218b6533a3f8bc\",\"vout\":9,\"address\":\"bc1qz79t3ntslq79g8sr9kj3keu8kczr450hkdwu3u\",\"label\":\"legacy\",\"scriptPubKey\":\"0014178ab8cd70f83c541e032da51b6787b6043ad1f7\",\"amount\":0.11786957,\"confirmations\":87004,\"spendable\":false,\"solvable\":false,\"reused\":false,\"safe\":true}]");
    JsonObject json(response);

    BOOST_CHECK(json.getChildAt(2).getChildAsString("txid") == "9ef1a4475188ab870ab6d768a9e7fb83cb2022b9a89ae296f8f998323fc4a06c");
    BOOST_CHECK(json.length() == 6);
}

BOOST_AUTO_TEST_SUITE_END()
