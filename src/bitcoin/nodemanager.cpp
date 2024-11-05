#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <algorithm>

#include "nodemanager.h"
#include "jsonparser.h"
#include "stringparser.h"
#include <thread>
#include <unistd.h>


string NodeManager::exec(const char* cmd) {
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
};

/*
string NodeManager::runCommand(const char* bitcoinCliCommand, bool clearFormatting) {
    std::string commandOutput = exec(bitcoinCliCommand);
    if (clearFormatting) {
        commandOutput.erase(
        std::remove_if(
            commandOutput.begin(), commandOutput.end(), [](char const c) {
                return '*' == c || ' ' == c || '%' == c || '\n' == c;
            }),
            commandOutput.end()
        );
    }
    
    return commandOutput;
}
*/

bool NodeManager::backgroundExecute(const char* method, const string params, const string path) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer("");
    string error("");

    string postFields = ("{\"jsonrpc\":\"1.0\",\"id\":\"curltest\",\"method\":\"");
    postFields.append(string(method)).append("\",\"params\":").append(params).append("}");
    cout << "Parms is " << postFields << endl;
    // Initialize cURL
    curl = curl_easy_init();
    
    if (curl) {
        // Set the URL for the Bitcoin Core API endpoint
        string uriPath = string("http://testclient:B1tc01n!@192.168.1.200:8332/").append(path);
        cout << "URI is " << uriPath << endl;
        //cout << endl << "http://testclient:B1tc01n!@192.168.1.200:8332" + path << endl;
        curl_easy_setopt(curl, CURLOPT_URL, uriPath.c_str());
        // Specify the POST request data (JSON-RPC request)
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        // Specify the write callback function to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 360);

        // Set headers
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            apiResponse = string(curl_easy_strerror(res));
        }

        JsonObject jsonObject(readBuffer);

        //JsonObject result = jsonObject.getChildAsJsonObject("result");
        //cout << readBuffer << endl;


        // Clean up
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        // cout << "Response here " << endl << readBuffer << endl;

        apiResponse = readBuffer;
        
        readBuffer.clear();
    }

    JsonObject jsonResponse = JsonObject(apiResponse);
    error = jsonResponse.getChildAsString("error");
    if(error != "null") {
        cout << "API error " << error << endl;
    }

    _operationInProgress.store(false);
    return (error == "null");
}

bool NodeManager::runApi(const char* method, const string params, const string path, const bool asBackgroundThread)
{
    apiResponse.clear();
    if (asBackgroundThread) {
        _operationInProgress.store(true);
        apiThread = std::thread(&NodeManager::backgroundExecute, this, method, params, path);
        return false;
    } else {
        return backgroundExecute(method, params, path);
    }
}

size_t NodeManager::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool NodeManager::operationIsInProgress() const {
    return _operationInProgress.load() && apiThread.joinable();
}

string NodeManager::internalCreateTransaction(CAmount amountSats, string address, string changeAddress)
{
  if (!runApi(LIST_UNSPENT, "[]", string("wallet/watchonly"), false)) {
    return "";
  }

  JsonObject unspentTxs(apiResponse);
  JsonObject resultString = unspentTxs.getChildAsString("result");
  JsonObject result = JsonObject(resultString);

  int i = 0;
  CAmount usedSats = 0;
  string inputs("[");
  do { // TODO: result.length() is the size in bytes, leading to crash. It should be the number of objects
    JsonObject input;
    JsonObject tx = result.getChildAt(i);
    CAmount txAmount = tx.getChildAsSatsAmount("amount");
    cout << endl << txAmount;
    usedSats += txAmount;
    input.addKVString("txid", tx.getChildAsString("txid"));
    input.addKVInt("vout", tx.getChildAsInt("vout"));
    inputs += input.toJson();
    if (usedSats < amountSats) {
      inputs += ",";
    }

    i++;

  } while (i < result.length() && usedSats <= amountSats);

  if (usedSats < amountSats) {
    cout << "Insufficient funds";
    return "Insufficient funds";
  }

  inputs += "]";

  //cout << "\nUsing " << i << " TXs, total amount " << usedSats;
  //cout << "\nInputs are > " << inputs;

  JsonObject output;
  output.addKVAmount(address, amountSats);

  // relayFee must met the minimum amount, in order for tx to be accepted
  // TODO: Loop to find minimum fee
  //CAmount relayFee = calculateFees();
  CAmount relayFee = i * 68 + 2 * 34; // 68 bytes is the size of a P2WPKH input, 34 bytes of an output
  output.addKVAmount(changeAddress, usedSats - amountSats - relayFee);
  string outputs = output.toJson();
  //cout << "\nOutputs are > " << outputs << endl;

  string parameters = inputs + "," + outputs;

  string escaped = escapeQuotes(parameters);

  if (!runApi(CREATE_RAW_TRANSACTION, string("[").append(parameters).append("]"), string("wallet/watchonly"), false)) {
    return "";
  }

  JsonObject createTransactionResponse(apiResponse);

  string rawTx = createTransactionResponse.getChildAsString("result");

  if (!runApi(DECODE_RAW_TRANSACTION, string("[\"").append(rawTx).append("\"]"), string("wallet/watchonly"), false)) {
    return "";
  }

  runApi(LOAD_WALLET, "[\"safe\"]", "", false);
  string error = JsonObject(apiResponse).getChildAsString("error");
  if (error != "null") {
    int code = JsonObject(error).getChildAsInt("code");
    if(code != -35) { // -35 = Wallet already loaded, it's fine
        return "Error loading wallet";
    }
  }
  
  if (!runApi(WALLET_PASSPHRASE, "[\"lara\", 60]", "wallet/safe", false)) {
    return "";
  }
  if (!runApi(SIGN_RAW_TRANSACTION_WITH_WALLET, "[\"" + rawTx + "\"]", "wallet/safe", false)) {
    return "";
  }

  JsonObject signResponse = JsonObject(apiResponse).getChildAsJsonObject("result");
  bool complete = signResponse.getChildAsBool("complete");
  if (!complete) {
    // TODO: Fix Json parsing to show error
    cout << "Sign error: " << signResponse.getChildAsString("errors");
    return "";
  }
  string signedTx = signResponse.getChildAsString("hex");
  if (!runApi(DECODE_RAW_TRANSACTION, string("[\"") + signedTx + "\"]", "", false)) {
    return "";
  }

  string decodedSignedTx = apiResponse;

  if (!runApi(TEST_MEMPOOL_ACCEPT, "[[\"" + signedTx + "\"]]", "", false)) {
    return "";
  }

  JsonObject mempoolAcceptResponse = JsonObject(apiResponse);
  error = mempoolAcceptResponse.getChildAsString("error");
  if (error != "null") {
    cout << "Transaction rejected with errors: " << apiResponse << endl;
    return "";
  }

  cout << endl;
  cout << "Transaction Valid! Check the content before confirmation: " << endl;
  cout << "\nCalculated fees > " << relayFee << "sats" << endl;
  cout << decodedSignedTx << endl;
  _operationInProgress.store(false);
  return "";
}

string NodeManager::sendAmountToAddress(CAmount amountSats, string address, string changeAddress) {
    apiResponse.clear();
    _operationInProgress.store(true);
    apiThread = std::thread(&NodeManager::internalCreateTransaction, this, amountSats, address, changeAddress);
    return "";
}

int NodeManager::calculateFees(const uint8_t txSize, uint8_t nBlocks) {
    if (!runApi(ESTIMATE_SMART_FEE, string("[") + to_string(nBlocks) + ", conservative]", "", false)) {
        return 0;
    }

    int feeKbyte = JsonObject(apiResponse).getChildAsSatsAmount("feerate");
    return feeKbyte * txSize * COIN / 1024;
}
