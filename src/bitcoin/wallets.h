// Utils to handle wallets

#ifndef WALLETS
#define WALLETS

#include "executor.h"
#include "amount.h"
#include "jsonparser.h"
#include "../crypto/key.h"

#define WIF_KEY_SIZE 53

using namespace std;

class WalletMgr: public Executor
{

private:
    static const char* CMD_LIST_WALLETS;
    static const char* CMD_GET_WALLET_INFO;
    static const char* CMD_GET_BALANCES;
    static const char* CMD_CREATE_WALLET;
    static const char* CMD_SETPASSPHRASE;
    static const char* CMD_SETHDSEED;
    static const char* CMD_LIST_UNSPENT;
    static const char* CMD_LIST_TRANSACTIONS;
    static const char* CMD_CREATE_RAW_TRANSACTION;
    static const char* CMD_FUND_RAW_TRANSACTION;
    static const char* CMD_DECODE_RAW_TRANSACTION;
    static const char* CMD_SIGN_RAW_TRANSACTION;
    static const char* CMD_SEND_RAW_TRANSACTION;
    static const char* CMD_TEST_MEMPOOL_ACCEPT;
    static const char* CMD_GET_CHANGE_ADDRESS;
    static const char* CMD_GET_NEW_ADDRESS;
    static const char* CMD_GET_ADDRESS_INFO;
    static const char* CMD_IMPORT_MULTI;
    static const char* CMD_ESTIMATE_FEE;
    static const char* CMD_SET_TX_FEE;


    bool hdSeedSet = false;

    string pwd;

    string _name;

    Key _masterKey;

    vector<string> pubkey_descriptors;
    vector<string> privkey_descriptors;
    
    WalletMgr(string walletName, uint8_t* masterSeed) { 
        _name = walletName;
        _masterKey = Key::MakeMasterKey(masterSeed, ENTROPY_SIZE);
    };

    string executeForWallet(const char* command);

    void setHDSeed(char* wifKey);

    std::optional<JsonObject> listUnspent();

    JsonObject listTransactions();

    string createRawTransaction(string inputs, string outputs);

    string fundRawTransaction(string transaction, string options);

    JsonObject getAddressInfo(string address);

    string createDescriptorImport(string descriptor, string label);

public:

    WalletMgr(string name){ _name = name; };
    
    bool createWallet(string password, bool watchonly);

    static WalletMgr Make(string walletName, uint8_t* masterSeed);

    void setMasterSeed(uint8_t* masterSeed);

    void unlockWallet();

    CAmount getBalance(string walletName);

    bool createSafeWallet(string password, char* wifKey);

    string createTransaction(CAmount amountSats, string address, string changeAddress);

    bool isHdSeedSet();

    string getNewChangeAddress();

    string getNewAddress();

    string decodeRawTransaction(string transactionHex);

    string signTransaction(string fundedTransactionHex);

    string sendTransaction(string signedTransactionHex);

    bool verifyTransaction(string signedTransactionHex);

    CAmount estimateFee(int confirmationBlock);

    bool setTxFee(CAmount txFee);

    JsonObject getInfo();

    string getAddressDescriptor(string address);

    bool addressIsUsed(string address);

    string importDescriptor(string descriptor, string label);

    void getBlockchainInfo();

    optional<JsonObject> getDescriptorInfo(string descriptor);

    vector<string> getDescriptors();

    void generateAccountDescriptors(int accountNumber);

    Key exportMasterPrivKey();

    void generateLightningMasterKey(Key& lightningKey);

    const string getMasterFingerprint();

    void setName(string name);
};

#endif
