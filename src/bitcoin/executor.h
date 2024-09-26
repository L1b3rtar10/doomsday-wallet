// Executes shell scripts

#ifndef RUNNER
#define RUNNER

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <optional>
#include <curl/curl.h>

using namespace std;

class Executor
{
    private:
        CURL* curl;
        CURLcode res;
        std::string readBuffer;

        string exec(const char* cmd);

        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    protected:
        Executor() : readBuffer("") {}
        string runCommand(const char* bitcoinCliCommand, bool clearFormatting);

        string runApi(const char* method, const string params, string& response, const string path);
};

#endif
