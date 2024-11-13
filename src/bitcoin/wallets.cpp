#include <iostream>
#include <string>
#include "jsonparser.h"
#include "executor.h"
#include "wallets.h"

const char* WalletMgr::CMD_LIST_WALLETS = "bitcoin-cli listwallets";
const char* WalletMgr::CMD_GET_WALLET_INFO = "getwalletinfo";
const char* WalletMgr::CMD_GET_BALANCES = "getbalances";
const char* WalletMgr::CMD_CREATE_WALLET = "bitcoin-cli createwallet";
const char* WalletMgr::CMD_SETPASSPHRASE = "walletpassphrase";
const char* WalletMgr::CMD_SETHDSEED = "sethdseed";
const char* WalletMgr::CMD_LIST_UNSPENT = "listunspent";
const char* WalletMgr::CMD_LIST_TRANSACTIONS = "listtransactions";
const char* WalletMgr::CMD_CREATE_RAW_TRANSACTION = "createrawtransaction";
const char* WalletMgr::CMD_FUND_RAW_TRANSACTION = "fundrawtransaction";
const char* WalletMgr::CMD_DECODE_RAW_TRANSACTION = "decoderawtransaction";
const char* WalletMgr::CMD_SIGN_RAW_TRANSACTION = "signrawtransactionwithwallet";
const char* WalletMgr::CMD_SEND_RAW_TRANSACTION = "sendrawtransaction";
const char* WalletMgr::CMD_TEST_MEMPOOL_ACCEPT = "testmempoolaccept";
const char* WalletMgr::CMD_GET_NEW_ADDRESS = "getnewaddress";
const char* WalletMgr::CMD_GET_CHANGE_ADDRESS = "getrawchangeaddress";
const char* WalletMgr::CMD_GET_ADDRESS_INFO = "getaddressinfo";
const char* WalletMgr::CMD_IMPORT_MULTI = "importmulti";
const char* WalletMgr::CMD_ESTIMATE_FEE = "bitcoin-cli estimatesmartfee";
const char* WalletMgr::CMD_SET_TX_FEE = "settxfee";


WalletMgr WalletMgr::Make(string walletName, uint8_t* masterSeed)
{
  return WalletMgr(walletName, masterSeed);
}

void WalletMgr::setMasterSeed(uint8_t *masterSeed)
{
  _masterKey = Key::MakeMasterKey(masterSeed, ENTROPY_SIZE, BIP_32);
}

bool WalletMgr::createSafeWallet(string password, char* wifKey)
{
  createWallet(password, false);
  unlockWallet();
  setHDSeed(wifKey);
  return true;
}

string WalletMgr::createTransaction(CAmount amountSats, string address, string changeAddress)
{
  /* Manually creates transaction input
  std::optional<JsonObject> unspentTxs = listUnspent();
  if (!unspentTxs) {
    cout << "Unable to find unspent transactions!" << endl;
    return nullopt;
  }
  
  CAmount usedSats = 0;
  size_t txIndex = 0;
  string inputs("[");
  do {
    JsonObject input;
    JsonObject tx = JsonObject(unspentTxs->getChildAt(txIndex));
    usedSats += tx.getChildAsSatsAmount("amount");
    input.addKVString("txid", tx.getChildAsString("txid"));
    input.addKVAmount("vout", tx.getChildAsInt("vout"));
    inputs += input.toJson();
    if (usedSats < amountSats) {
      inputs += ",";
    }
    
    txIndex++;
    cout << "\nUsed sats > " << usedSats << " totalAmount > " << amountSats;
  } while (usedSats <= amountSats && txIndex < unspentTxs->length());

  inputs += "]";

  cout << "\nUsing " << txIndex << " TXs, total amount " << usedSats;
  cout << "\nInputs are > " << inputs;
  */

  string inputs("[]");
  JsonObject output;
  double amountBtc = (double)amountSats/COIN;
  output.addKVString(address, to_string(amountBtc));
  string outputs = "[" + output.toJson() + "]";
  cout << "\nOutputs are > " << outputs;

  string rawTransaction = createRawTransaction(inputs, outputs);
  cout << "Raw transaction ready > " << rawTransaction << endl;

  JsonObject fundOptions;
  fundOptions.addKVString("changeAddress", changeAddress);

  return fundRawTransaction(rawTransaction, fundOptions.toJson());
}

bool WalletMgr::isHdSeedSet() {
  return hdSeedSet;
}

string WalletMgr::getAddressDescriptor(string address)
{
  string descriptor = getAddressInfo(address).getChildAsString("desc");
  descriptor.erase(
    std::remove_if(
        descriptor.begin(), descriptor.end(), [](char const c) {
            return '"' == c;
        }),
        descriptor.end()
    );
  return descriptor;
}

bool WalletMgr::addressIsUsed(string address)
{
  JsonObject transactions = listTransactions();
  for (size_t i = 0; i < transactions.length(); i++) {
    JsonObject transaction = JsonObject(transactions.getChildAt(i));
    string usedAddress = transaction.getChildAsString("address");
    usedAddress.erase(
    std::remove_if(
        usedAddress.begin(), usedAddress.end(), [](char const c) {
            return '"' == c;
        }),
        usedAddress.end()
    );
    if (address == usedAddress) {
        return true;
    }
  }
  return false;
}

// https://developer.bitcoin.org/reference/rpc/getbalances.html
CAmount WalletMgr::getBalance(string walletName)
{
  std::optional<string> response = runCommand(executeForWallet(CMD_GET_BALANCES).c_str(), false);
  JsonObject jsonOutput(response->c_str());
  JsonObject wallet = jsonOutput.getChildAsJsonObject(walletName);
  return wallet.getChildAsSatsAmount("trusted");
}

// https://developer.bitcoin.org/reference/rpc/getwalletinfo.html
JsonObject WalletMgr::getInfo()
{
  std::optional<string> response = runCommand(executeForWallet(CMD_GET_WALLET_INFO).c_str(), false);
  return JsonObject(response->c_str());
}

// https://bitcoincore.org/en/doc/26.0.0/rpc/wallet/createwallet/
bool WalletMgr::createWallet(string password, bool watchonly)
{
  string response = string();
  string params;

  if (watchonly) {
    params = string("{ \
              \"wallet_name\":\""+ _name + "\", \
              \"blank\":true, \
              \"avoid_reuse\":true, \
              \"descriptors\":true, \
              \"load_on_startup\":true, \
              \"disable_private_keys\":true }");
  } else {
    params = string("{ \
              \"wallet_name\":\""+ _name + "\", \
              \"blank\":true, \
              \"passphrase\":\"" + password + "\", \
              \"avoid_reuse\":true, \
              \"descriptors\":true, \
              \"load_on_startup\":true, \
              \"disable_private_keys\":false }");
  }
  runApi("createwallet", params, response, "/");
  return true;
}

// https://developer.bitcoin.org/reference/rpc/walletpassphrase.html
void WalletMgr::unlockWallet()
{
  string command = executeForWallet(CMD_SETPASSPHRASE) + std::string(" \"") + pwd + std::string("\" 60");
  std::optional<JsonObject> response = runCommand(command.c_str(), true);
}

// https://developer.bitcoin.org/reference/rpc/sethdseed.html
void WalletMgr::setHDSeed(char* wifKey)
{
  string command = executeForWallet(CMD_SETHDSEED) + std::string(" true \"") + std::string(wifKey) + std::string("\"");
  std::optional<JsonObject> response = runCommand(command.c_str(), true);
  hdSeedSet = true;
}

void WalletMgr::generateAccountDescriptors(int accountNumber)
{
  char descriptor[MAX_DESCRIPTOR_LENGTH] = {'\0'};
  string output;

  _masterKey.exportAccountDescriptor(BIP_32, accountNumber, RECEIVING, descriptor);
  output = "pkh(" + string(descriptor) + ")";
  pubkey_descriptors.push_back(output);
  memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

  _masterKey.exportAccountDescriptor(BIP_32, accountNumber, CHANGE, descriptor);
  output = "pkh(" + string(descriptor) + ")";
  pubkey_descriptors.push_back(output);
  memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

  _masterKey.exportAccountDescriptor(BIP_49, accountNumber, RECEIVING, descriptor);
  output = "sh(wpkh(" + string(descriptor) + "))";
  pubkey_descriptors.push_back(output);
  memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

  _masterKey.exportAccountDescriptor(BIP_49, accountNumber, CHANGE, descriptor);
  output = "sh(wpkh(" + string(descriptor) + "))";
  pubkey_descriptors.push_back(output);
  memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

  _masterKey.exportAccountDescriptor(BIP_84, accountNumber, RECEIVING, descriptor);
  output = "wpkh(" + string(descriptor) + ")";
  pubkey_descriptors.push_back(output);
  memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

  _masterKey.exportAccountDescriptor(BIP_84, accountNumber, CHANGE, descriptor);
  output = "wpkh(" + string(descriptor) + ")";
  pubkey_descriptors.push_back(output);
  memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

  _masterKey.exportAccountDescriptor(BIP_86, accountNumber, RECEIVING, descriptor);
  output = "tr(" + string(descriptor) + ")";
  pubkey_descriptors.push_back(output);
  memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);

  _masterKey.exportAccountDescriptor(BIP_86, accountNumber, CHANGE, descriptor);
  output = "tr(" + string(descriptor) + ")";
  pubkey_descriptors.push_back(output);
  memset(descriptor, '\0', MAX_DESCRIPTOR_LENGTH);
}

Key WalletMgr::exportMasterPrivKey()
{
  return _masterKey;
}

// https://developer.bitcoin.org/reference/rpc/listunspent.html
std::optional<JsonObject> WalletMgr::listUnspent()
{
  string command = executeForWallet(CMD_LIST_UNSPENT);
  string response = runCommand(command.c_str(), true);
  return JsonObject(response);
}

// https://developer.bitcoin.org/reference/rpc/listtransactions.html
JsonObject WalletMgr::listTransactions()
{
  string command = executeForWallet(CMD_LIST_TRANSACTIONS);
  return JsonObject(runCommand(command.c_str(), false));
}

// https://developer.bitcoin.org/reference/rpc/createrawtransaction.html
string WalletMgr::createRawTransaction(string inputs, string outputs)
{
  //string command = executeForWallet(CMD_CREATE_RAW_TRANSACTION) + " \"" + inputs + "\" \"" + outputs + "\"";
  string command = executeForWallet(CMD_CREATE_RAW_TRANSACTION) + " \'[]\' \'" + outputs + "\'";
  return runCommand(command.c_str(), true);
}

// https://developer.bitcoin.org/reference/rpc/fundrawtransaction.html
string WalletMgr::fundRawTransaction(string transactionHex, string options)
{
  string command = executeForWallet(CMD_FUND_RAW_TRANSACTION) + std::string(" \'") + transactionHex + std::string("\' \'") + options + std::string("\'");
  return runCommand(command.c_str(), true);
}

// https://developer.bitcoin.org/reference/rpc/decoderawtransaction.html
string WalletMgr::decodeRawTransaction(string transactionHex)
{
  string command = executeForWallet(CMD_DECODE_RAW_TRANSACTION) + " \'" + transactionHex + "\'";
  return runCommand(command.c_str(), false);
}

// https://developer.bitcoin.org/reference/rpc/signrawtransactionwithwallet.html
string WalletMgr::signTransaction(string fundedTransactionHex)
{
  string command = executeForWallet(CMD_SIGN_RAW_TRANSACTION) + " " + fundedTransactionHex;
  return runCommand(command.c_str(), false);
}

// https://developer.bitcoin.org/reference/rpc/testmempoolaccept.html
bool WalletMgr::verifyTransaction(string signedTransactionHex)
{
  string command = executeForWallet(CMD_TEST_MEMPOOL_ACCEPT) + std::string(" [\'") + signedTransactionHex + std::string("\']");
  string response = runCommand(command.c_str(), false);
  JsonObject jsonResponse = JsonObject(response);
  JsonObject transaction = jsonResponse.getChildAt(0);
  if (transaction.getChildAsString("allowed") == "true") {
    return true;
  } else {
    string error = transaction.getChildAsString("reject-reason");
    cout << "Validation failed > " << error;
  }

  return false;
}

// https://developer.bitcoin.org/reference/rpc/signrawtransactionwithwallet.html
string WalletMgr::sendTransaction(string signedTransactionHex)
{
  string command = executeForWallet(CMD_SEND_RAW_TRANSACTION) + " \'" + signedTransactionHex + "\'";
  return runCommand(command.c_str(), false);
}

// https://developer.bitcoin.org/reference/rpc/getrawchangeaddress.html
string WalletMgr::getNewChangeAddress()
{
  string command = executeForWallet(CMD_GET_CHANGE_ADDRESS);
  return runCommand(command.c_str(), true);
}

// https://developer.bitcoin.org/reference/rpc/getnewaddress.html
string WalletMgr::getNewAddress()
{
  string command = executeForWallet(CMD_GET_NEW_ADDRESS);
  return runCommand(command.c_str(), true);
}

// https://https://developer.bitcoin.org/reference/rpc/importdescriptors.html
string WalletMgr::importDescriptor(string descriptor, string label)
{
  string command = createDescriptorImport(descriptor, label);
  string response = string();
  runApi("importdescriptors", "[" + command + "]", response, string("/wallet/").append(_name));
  return response;
}

// https://developer.bitcoin.org/reference/rpc/getaddressinfo.html
JsonObject WalletMgr::getAddressInfo(string address)
{
  string command = executeForWallet(CMD_GET_ADDRESS_INFO) + " \"" + address + "\"";
  string response = runCommand(command.c_str(), true);
  return JsonObject(response);
}

// https://developer.bitcoin.org/reference/rpc/estimatesmartfee.html
CAmount WalletMgr::estimateFee(int confirmationBlock)
{
  string estimateFeeCmd = std::string(CMD_ESTIMATE_FEE) + std::string(" ") + std::to_string(confirmationBlock) + std::string(" economical");
  string response = runCommand(estimateFeeCmd.c_str(), true);
  JsonObject jsonResponse = JsonObject(response);
  return jsonResponse.getChildAsSatsAmount("feerate");
}

// https://developer.bitcoin.org/reference/rpc/settxfee.html
bool WalletMgr::setTxFee(CAmount txFee)
{
  string setFeeCmd = executeForWallet(CMD_SET_TX_FEE) + " " + std::to_string((double)txFee/COIN);
  string response = runCommand(setFeeCmd.c_str(), true);
  return response == "true";
}

string WalletMgr::executeForWallet(const char* command)
{
  return std::string("bitcoin-cli -rpcwallet=") + _name + std::string(" ") + std::string(command);
}

void WalletMgr::getBlockchainInfo()
{
  string response = string();
  runApi("getblockchaininfo", "[]", response, "/");
  getchar();
}

optional<JsonObject> WalletMgr::getDescriptorInfo(string descriptor)
{
    string response = string();
    runApi("getdescriptorinfo", "[\"" + descriptor + "\"]", response, string("/wallet/").append(_name));

    JsonObject responseJson = JsonObject(response);
    string error = responseJson.getChildAsString("error");
    if (error == "null") {
      return responseJson.getChildAsJsonObject("result");
    } else {
      cout << "An error occurred > " << error << endl;
      return nullopt;
    }
}

vector<string> WalletMgr::getDescriptors()
{
    return pubkey_descriptors;
}

string WalletMgr::createDescriptorImport(string descriptor, string label)
{
  JsonObject input;
  
  bool isChangeAddress = descriptor.find("/1/*") != string::npos;

  input.addKVString("desc", descriptor.substr(1, descriptor.length() - 2));
  input.addKVString("timestamp", "now");
  input.addKVBool("internal", isChangeAddress);
  input.addKVBool("active", true);
  if (!isChangeAddress) {
    input.addKVString("label", label);
  }
  if (!isChangeAddress && label.size() > 0) {
    input.addKVString("label", label);
  }
  
  return "[" + input.toJson() + "]";
}

const string WalletMgr::getMasterFingerprint() {
  return _masterKey.getFingerprint();
}

void WalletMgr::setName(string name) {
  _name = name;
}
