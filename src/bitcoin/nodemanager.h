#ifndef NODE_MANAGER
#define NODE_MANAGER

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <optional>
#include <thread>
#include <atomic>
#include <curl/curl.h>
#include "amount.h"

using namespace std;

static const char* GET_BLOCKCHAIN_INFO = "getblockchaininfo";
static const char* LIST_WALLETS = "listwallets";
static const char* LIST_DESCRIPTORS = "listdescriptors";
static const char* GET_DESCRIPTOR_INFO = "getdescriptorinfo";
static const char* IMPORT_DESCRIPTORS = "importdescriptors";
static const char* GET_WALLET_INFO = "getwalletinfo";
static const char* LOAD_WALLET = "loadwallet";
static const char* WALLET_PASSPHRASE = "walletpassphrase";
static const char* CREATE_WALLET = "createwallet";
static const char* LIST_UNSPENT = "listunspent";
static const char* CREATE_RAW_TRANSACTION = "createrawtransaction";
static const char* DECODE_RAW_TRANSACTION = "decoderawtransaction";
static const char* FUND_RAW_TRANSACTION = "fundrawtransaction";
static const char* SIGN_RAW_TRANSACTION_WITH_WALLET = "signrawtransactionwithwallet";
static const char* TEST_MEMPOOL_ACCEPT = "testmempoolaccept";
static const char* SEND_TO_ADDRESS = "sendtoaddress";
static const char* ESTIMATE_SMART_FEE = "estimatesmartfee";


class NodeManager {

    public:
        NodeManager(){ 
            apiResponse = string("");
            _operationInProgress = true;
        };

        bool operationIsInProgress() const;

        bool runApi(const char* method, const string params, const string path, const bool asBackgroundThread);

        string apiResponse;

        std::thread apiThread;

        string sendAmountToAddress(CAmount amountSats, string address, string changeAddress);

        void createWatchonlyWallet(const string walletName);

        void importDescriptors(const string walletName, const vector<string> descriptors);

        void internalImportDescriptors(const string walletName, const vector<string> descriptors);

    private:

        std::atomic<bool> _operationInProgress;

        string exec(const char* cmd);

        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

        bool backgroundExecute(const char* method, const string params, const string path);

        string internalCreateTransaction(CAmount amountSats, string address, string changeAddress);

        int calculateFees(const uint8_t txSize, uint8_t nBlocks);
};

#endif
