#include "jsonparser.h"
#include "stringparser.h"


bool JsonObject::isArray()
{
  return _jsonString[0] == '[';
}

size_t JsonObject::length() {
    return valueIndex.size();
}

string JsonObject::getKeyAt(size_t index) {
    return keys[index];
}

bool JsonObject::hasKey(string key) {
  for (int i = 0; i < keys.size(); i++) {
    if (key.compare(keys[i]) == 0) {
      return true;
    }
  }
  return false;
}

string JsonObject::getChildAsString(string key) {
    for (int i = 0; i < keys.size(); i++) {
      if (key.compare(keys[i]) == 0) {
        string value = _jsonString.substr(valueIndex[i], valueSize[i]);
        return value;
      }
    }
    return "";
}

JsonObject JsonObject::getChildAsJsonObject(string key) {
  return JsonObject(getChildAsString(key));
}

CAmount JsonObject::getChildAsSatsAmount(string key) {
  for (int i = 0; i < keys.size(); i++) {
      if (key.compare(keys[i]) == 0) {
        CAmount amount;
        ParseFixedPoint(_jsonString.substr(valueIndex[i], valueSize[i]), 8, &amount);
        return amount;
      }
    }
    return 0;
}

int JsonObject::getChildAsInt(string key) {
  for (int i = 0; i < keys.size(); i++) {
      if (key.compare(keys[i]) == 0) {
        string value = _jsonString.substr(valueIndex[i], valueSize[i]);
        return stoi(value);
      }
    }
    return 0;
}

string JsonObject::getChildAt(size_t index) {
  return _jsonString.substr(valueIndex[index], valueSize[index]);
}

string JsonObject::print() {
  return _jsonString;
}

void JsonObject::addKVString(string key, string value) {
  keys.push_back(key);
  values.push_back(value);
  types.push_back(STRING);
}

void JsonObject::addKVAmount(string key, CAmount amount) {
  keys.push_back(key);
  values.push_back(to_string(amount));
  types.push_back(NUMBER);
}

void JsonObject::addKVInt(string key, int value) {
  keys.push_back(key);
  values.push_back(to_string(value));
  types.push_back(NUMBER);
}

void JsonObject::addKVBool(string key, bool value) {
  keys.push_back(key);
  values.push_back(value ? "true" : "false");
  types.push_back(BOOL);
}

string JsonObject::toJson() {
  // Generates Json formatted string
  string jsonString("{");
  for (int i = 0; i < keys.size(); i++) {
    string separator = "\"";
    if (types[i] == NUMBER || types[i] == BOOL) {
      separator = "";
    }
    
    jsonString = jsonString + "\"" + keys[i] + "\":" + separator + values[i] + separator;
    if (i < keys.size() - 1) {
      jsonString += ",";
    }
  }
  jsonString += "}";
  return jsonString;
}
