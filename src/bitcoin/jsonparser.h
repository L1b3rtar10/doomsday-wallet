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
    //vector<Node> nodes;
    vector<string> keys;
    vector<string> values;
    vector<JsonType> types;
    vector<int> valueIndex;
    vector<int> valueSize;

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

        if (isArray()) {
            if (_jsonString[1] != '{') {
                valueStart = 1;
                openKey = true;
                valueIndex.push_back(valueStart);
                //cout << "\nKey open";
            }
            
            for (int i = 0; i < _jsonString.length(); i++) {
                if (_jsonString[i] == '{') {
                    if (!openKey && depth == 0) {
                        valueStart = i;
                        openKey = true;
                        valueIndex.push_back(valueStart);
                        //cout << "\nKey open";
                    }
                    depth += 1;    
                } else if (_jsonString[i] == '}') {
                    depth -= 1;
                } else if ((_jsonString[i] == ',') && (depth == 0)) {
                    valueSize.push_back(i - valueStart);
                    //cout << "\nValue found " << valueIndex[valueIndex.size() - 1];
                    valueStart = i + 1;
                    valueIndex.push_back(valueStart);
                    openKey = true;
                } else if (_jsonString[i] == ']' && openKey && depth == 0) {
                    valueSize.push_back(i - valueStart);
                    //cout << "\nValue found " << valueIndex[valueIndex.size() - 1];
                }
                
                //cout << "\n" << _jsonString[i] << " depth > " << unsigned(depth);
            }
        } else {
            for (int i = 0; i < _jsonString.length(); i++) {
                if (_jsonString[i] == '"' && !openKey && depth == 0 && !isLeaf) {
                    keyStart = i + 1;
                    openKey = true;
                } else if (_jsonString[i] == '"' && openKey && !isLeaf) {
                    openKey = false;
                    keys.push_back(_jsonString.substr(keyStart, i - keyStart));
                    //cout << "\nKey found " << keys[keys.size() - 1];
                } else if (_jsonString[i] == '}') {
                    if (depth == 1) {
                        depth -= 1;
                        valueSize.push_back(i + 1 - valueStart);
                    } else if (depth == 0) {
                        valueSize.push_back(i - valueStart);
                    } else {
                        depth -= 1;
                    }
                } else if (_jsonString[i] == ':') {
                    if (_jsonString[i + 1] == '{' && depth == 0) {
                        depth += 1;
                        valueStart = i + 1;
                        valueIndex.push_back(valueStart);
                    } else if (depth == 0) {
                        isLeaf = true;
                        valueStart = i + 1;
                        valueIndex.push_back(valueStart);
                        //cout << "\nValue found " << valueIndex[valueIndex.size() - 1];
                    }
                } else if (_jsonString[i] == ',' && isLeaf) {
                    valueSize.push_back(i - valueStart);
                    //cout << "\nLeaf closed " << valueSize[valueSize.size() - 1];
                    isLeaf = false;
                }
                
                //cout << "\n" << _jsonString[i] << " depth>" << " " << unsigned(depth) << " isLeaf>" << isLeaf;
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

    bool hasKey(string key);

    string getChildAt(size_t index);

    string print();

    void addKVString(string key, string value);

    void addKVInt(string key, int value);

    void addKVBool(string key, bool value);

    void addKVAmount(string key, CAmount amount);

    string toJson();
};

/*class Node
{
private:
    string _key;
    string _value;
public:
    Node(key, value) { _key = key; _value = value};
}*/

#endif
