// Utils for Json data

#ifndef JSON
#define JSON

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "amount.h"

using namespace std;

enum JsonType {
    STRING = 0,
    NUMBER = 1,
    BOOL = 2,
    OBJ = 3,
};

class JsonObject
{
private:
    string _jsonString;
    vector<string> keys;
    vector<string> values;
    vector<JsonType> types;
    vector<size_t> valueIndex;
    vector<size_t> valueSize;

    char setClosingChar(char openChar);

public:

    JsonObject() {
        _jsonString = string("");
    };

    JsonObject(string jsonString) {
        jsonString.erase(
        std::remove_if(
            jsonString.begin(), jsonString.end(), [](char const c) {
                return ' ' == c || '%' == c || '\n' == c;
            }),
            jsonString.end()
        );
        _jsonString = jsonString;
        
        bool openKey = false;
        bool openValue = false;
        int keyStart = 0;
        int valueStart = 0;
        uint8_t depth = 0;
        char closingChar = '\0';
        char openChar = '\0';

        if (isArray()) {
//          cout << "Is array " << _jsonString.length() << endl;
            for (size_t i = 0; i < _jsonString.length(); i++) {
//              cout << _jsonString[i] << " depth > " << to_string(depth) << endl;
                if (_jsonString[i] == '{') {
//                  cout << "Open" << endl;
                    depth += 1; 
                    if (depth == 1) {
                        valueStart = i;
                        valueIndex.push_back(valueStart);
                    }
                } else if (_jsonString[i] == '}') {
//                  cout << "Close" << endl;
                    depth -= 1;
                } else if ((_jsonString[i] == ',' || (_jsonString[i] == ']')) && (depth == 0)) {
                    valueSize.push_back(i - valueStart);
                }
            }
        } else {
//          cout << _jsonString.length() << endl;
            for (size_t i = 0; i < _jsonString.length(); i++) {
//              cout << _jsonString[i] << " depth > " << to_string(depth) << endl;
                if (_jsonString[i] == '"' && !openKey && !openValue && depth == 0) {
                    keyStart = i + 1;
                    openKey = true;
//                  cout << "openKey" << endl;
                } else if (_jsonString[i] == '"' && openKey) {
                    openKey = false;
                    keys.push_back(_jsonString.substr(keyStart, i - keyStart));
//                  cout << "closeKey > " << keys[keys.size() - 1] << endl;
                } else if ((_jsonString[i] == closingChar) && (i != valueIndex[valueIndex.size() - 1])) {
                    depth -= 1;
//                  cout << "depth - 1" << endl;
                    if (depth == 0) {
                        closingChar = '\0';
                    }
                } else if (_jsonString[i] == ':' && depth == 0) {
                    valueStart = i + 1;
                    openValue = true;
                    valueIndex.push_back(valueStart);
                    if (_jsonString[valueStart] == '{' || _jsonString[valueStart] == '[' || _jsonString[valueStart] == '"') {
                        openChar = _jsonString[valueStart];
                        closingChar = setClosingChar(_jsonString[valueStart]);
                    }
//                  cout << "openValue" << endl;
                } else if ((_jsonString[i] == ',' || _jsonString[i] == closingChar || _jsonString[i] == '}') && depth == 0 && openValue && (i != valueIndex[valueIndex.size() - 1])) {
                    valueSize.push_back(i - valueStart);
                    openValue = false;
//                  cout << "new value " << _jsonString.substr(valueIndex[valueIndex.size()-1], valueSize[valueSize.size()-1]) << endl;
                } else if (_jsonString[i] == openChar) {
                    depth += 1;
//                  cout << "depth + 1, same open char > " << openChar << endl;
                }
            }
        }
    };

    bool isArray();

    size_t length();

    string getKeyAt(size_t index);

    string getChildAsString(string key);

    JsonObject getChildAsJsonObject(string key);

    CAmount getChildAsSatsAmount(string key);

    int getChildAsInt(string key);

    uint64_t getChildAsBigInt(string key);

    bool getChildAsBool(string key);

    bool hasKey(string key);

    JsonObject getChildAt(size_t index);

    string print();

    void addKVString(string key, string value);

    void addKVInt(string key, uint64_t value);

    void addKVBool(string key, bool value);

    void addKVAmount(string key, CAmount amountSats);

    void addJsonObject(JsonObject value);

    string toJson();
};

#endif
