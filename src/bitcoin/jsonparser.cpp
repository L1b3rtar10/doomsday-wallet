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
  for (size_t i = 0; i < keys.size(); i++) {
    if (key.compare(keys[i]) == 0) {
      return true;
    }
  }
  return false;
}

string JsonObject::getChildAsString(string key) {
    for (size_t i = 0; i < keys.size(); i++) {
      if (key.compare(keys[i]) == 0) {
        bool hasQuotes = _jsonString[valueIndex[i]] == '"' && _jsonString[valueIndex[i] + valueSize[i] - 1] == '"';
        string value = _jsonString.substr(valueIndex[i] + (hasQuotes ? 1 : 0), valueSize[i] - (hasQuotes ? 2 : 0));
        return value;
      }
    }
    return "";
}

bool JsonObject::getChildAsBool(string key) {
  for (size_t i = 0; i < keys.size(); i++) {
      if (key.compare(keys[i]) == 0) {
        string value = _jsonString.substr(valueIndex[i], valueSize[i]);
        if (value == "true") {
          return true;
        }
      }
    }
    return false;
}

JsonObject JsonObject::getChildAsJsonObject(string key) {
  return JsonObject(getChildAsString(key));
}

CAmount JsonObject::getChildAsSatsAmount(string key) {
  for (size_t i = 0; i < keys.size(); i++) {
      if (key.compare(keys[i]) == 0) {
        CAmount amount;
        ParseFixedPoint(_jsonString.substr(valueIndex[i], valueSize[i]), 8, &amount);
        return amount;
      }
    }
    return 0;
}

int JsonObject::getChildAsInt(string key) {
  for (size_t i = 0; i < keys.size(); i++) {
      if (key.compare(keys[i]) == 0) {
        string value = _jsonString.substr(valueIndex[i], valueSize[i]);
        return stoi(value);
      }
    }
    return 0;
}

uint64_t JsonObject::getChildAsBigInt(string key) {
  for (size_t i = 0; i < keys.size(); i++) {
      if (key.compare(keys[i]) == 0) {
        string value = _jsonString.substr(valueIndex[i], valueSize[i]);
        return stoll(value);
      }
    }
    return 0;
}

JsonObject JsonObject::getChildAt(size_t index) {
  return JsonObject(_jsonString.substr(valueIndex[index], valueSize[index]));
}

string JsonObject::print() {
  return _jsonString;
}

void JsonObject::addKVString(string key, string value) {
  keys.push_back(key);
  values.push_back(value);
  types.push_back(STRING);
}

void JsonObject::addKVAmount(string key, CAmount amountSats) {
  keys.push_back(key);
  double amountBtc = (double)amountSats/COIN;
  values.push_back(to_string(amountBtc));
  types.push_back(NUMBER);
}

void JsonObject::addKVInt(string key, uint64_t value) {
  keys.push_back(key);
  values.push_back(to_string(value));
  types.push_back(NUMBER);
}

void JsonObject::addKVBool(string key, bool value) {
  keys.push_back(key);
  values.push_back(value ? "true" : "false");
  types.push_back(BOOL);
}

void JsonObject::addJsonObject(JsonObject value) {
  keys.push_back("");
  values.push_back(value.toJson());
  types.push_back(OBJ);
}

string JsonObject::toJson() {
  // Generates Json formatted string
  string jsonString("{");

  for (size_t i = 0; i < keys.size(); i++) {
    if (types[i] == OBJ) {
      jsonString += values[i];
    } else {
      string escapeSequence = types[i] == STRING ? "\"": "";
      jsonString = jsonString + "\"" + keys[i] + "\":" + escapeSequence + values[i] + escapeSequence;
    }
    if (i < keys.size() - 1) {
      jsonString += ",";
    }
  }
  jsonString += "}";
  return jsonString;
}

char JsonObject::setClosingChar(char openChar) {
  if (openChar == '{') {
    return '}';
  } else if (openChar == '[') {
    return ']';
  } else if (openChar == '"') {
    return '"';
  }
  return ',';
}
