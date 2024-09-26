#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <algorithm>

#include "executor.h"
#include "jsonparser.h"


string Executor::exec(const char* cmd) {
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

string Executor::runCommand(const char* bitcoinCliCommand, bool clearFormatting) {
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

string Executor::runApi(const char* method, const string params, string& response, const string path)
{
    string postFields = ("{\"jsonrpc\":\"1.0\",\"id\":\"curltest\",\"method\":\"");
    postFields.append(string(method)).append("\",\"params\":").append(params).append("}");
    cout << "Parms is " << postFields << endl;
    // Initialize cURL
    curl = curl_easy_init();
    if (curl) {
        // Set the URL for the Bitcoin Core API endpoint
        string uriPath = string("http://testclient:B1tc01n!@192.168.1.200:8332").append(path);
        cout << endl << "http://testclient:B1tc01n!@192.168.1.200:8332" + path << endl;
        curl_easy_setopt(curl, CURLOPT_URL, uriPath.c_str());
        // Specify the POST request data (JSON-RPC request)
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        // Specify the write callback function to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Set headers
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        JsonObject jsonObject(readBuffer);

        //JsonObject result = jsonObject.getChildAsJsonObject("result");
        cout << readBuffer << endl;


        // Clean up
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        response = readBuffer;

        readBuffer.clear();

        return response;
    }

    return "An error occurred > curl";
}

size_t Executor::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
