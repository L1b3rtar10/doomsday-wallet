// Utils for Json data

#ifndef JSON
#define JSON

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "amount.h"

using namespace std;

typedef enum JsonType {
    STRING = 0,
    NUMBER = 1,
    BOOL = 2,
};

class JsonObject
{
private:
    string _jsonString;
    vector<string> keys;
    vector<string> values;
    vector<JsonType> types;
    vector<int> valueIndex;
    vector<int> valueSize;

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
        bool isLeaf = false;
        int keyStart = 0;
        int valueStart = 0;
        uint8_t depth = 0;
        char closingChar = '\0';

        if (isArray()) {
            for (int i = 0; i < _jsonString.length(); i++) {
                if (_jsonString[i] == '{') {
                    depth += 1; 
                    if (depth == 1) {
                        valueStart = i;
                        valueIndex.push_back(valueStart);
                    }
                } else if (_jsonString[i] == '}') {
                    depth -= 1;
                } else if ((_jsonString[i] == ',' || (_jsonString[i] == ']')) && (depth == 0)) {
                    valueSize.push_back(i - valueStart);
                }
            }
        } else {
            for (int i = 0; i < _jsonString.length(); i++) {
                if (_jsonString[i] == '"' && !openKey && depth == 0 && !isLeaf) {
                    keyStart = i + 1;
                    openKey = true;
                } else if (_jsonString[i] == '"' && openKey && !isLeaf) {
                    openKey = false;
                    keys.push_back(_jsonString.substr(keyStart, i - keyStart));
                } else if (_jsonString[i] == closingChar) {
                    if (depth == 1) {
                        depth -= 1;
                        valueSize.push_back(i + 1 - valueStart);
                    } else if (depth == 0) {
                        valueSize.push_back(i - valueStart);
                    } else {
                        depth -= 1;
                    }
                    closingChar = '\0';
                } else if (_jsonString[i] == ':') {
                    if ((_jsonString[i + 1] == '[' || _jsonString[i + 1] == '{') && depth == 0) {
                        depth += 1;
                        valueStart = i + 1;
                        valueIndex.push_back(valueStart);
                        closingChar = setClosingChar(_jsonString[i + 1]);
                    } else if (depth == 0 && closingChar == '\0') {
                        isLeaf = true;
                        valueStart = i + (_jsonString[i + 1] == '"' ? 2 : 1);
                        valueIndex.push_back(valueStart);
                    }
                } else if ((_jsonString[i] == ',' || _jsonString[i] == '}') && isLeaf) {
                    valueSize.push_back(i - (_jsonString[i - 1] == '"' ? 1 : 0) - valueStart);
                    isLeaf = false;
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

    bool getChildAsBool(string key);

    bool hasKey(string key);

    JsonObject getChildAt(size_t index);

    string print();

    void addKVString(string key, string value);

    void addKVInt(string key, int value);

    void addKVBool(string key, bool value);

    void addKVAmount(string key, CAmount amountSats);

    string toJson();
};

#endif
