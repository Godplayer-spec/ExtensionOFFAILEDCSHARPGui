#include "pch.h"
#include <Windows.h>
#include <iostream>
#include <curl/curl.h>
#include <string>
#include <json/json.h>

// Callback types and globals
typedef void(__stdcall* ProgressCallbackType)(int progress, const char* currentPath);
typedef void(__cdecl* ConsoleCallbackType)(const char* text);

ProgressCallbackType g_progressCallback = nullptr;
ConsoleCallbackType g_consoleCallback = nullptr;
bool g_isInitialized = false;

// Helper functions
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void LogMessage(const char* message) {
    if (g_consoleCallback) {
        g_consoleCallback(message);
    }
}

void queryapi() {
    LogMessage("Initializing CURL...");

    CURL* curl = curl_easy_init();
    if (curl == NULL) {
        LogMessage("CURL initialization failed.");
        return;
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000/regpaths");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

    LogMessage("Performing API request...");
    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        std::string error = "CURL error: ";
        error += curl_easy_strerror(result);
        LogMessage(error.c_str());
        curl_easy_cleanup(curl);
        return;
    }

    try {
        Json::Value root;
        Json::CharReaderBuilder readerBuilder;
        std::string errs;
        std::istringstream jsonStream(readBuffer);

        if (!Json::parseFromStream(readerBuilder, jsonStream, &root, &errs)) {
            LogMessage(("JSON Parse Error: " + errs).c_str());
            curl_easy_cleanup(curl);
            return;
        }

        const Json::Value paths = root["paths"];
        LogMessage("Paths received from API:");
        for (const auto& path : paths) {
            LogMessage(path.asString().c_str());
        }
    }
    catch (const std::exception& e) {
        LogMessage(("JSON Processing Error: " + std::string(e.what())).c_str());
    }

    curl_easy_cleanup(curl);
    LogMessage("API query completed.");
}

// Exported functions
extern "C" {
    __declspec(dllexport) void HelloFromCpp() {
        MessageBoxA(NULL, "Hello from C++!", "Debug", MB_OK);
    }

    __declspec(dllexport) void CleanRegistry(ProgressCallbackType callback) {
        g_progressCallback = callback;
    }

    __declspec(dllexport) void TestCallback() {
        if (g_consoleCallback) {
            g_consoleCallback("Test callback successful");
        }
    }

    __declspec(dllexport) void InitializeDLL() {
        if (!g_isInitialized) {
            CURLcode result = curl_global_init(CURL_GLOBAL_DEFAULT);
            if (result != CURLE_OK) {
                if (g_consoleCallback) {
                    g_consoleCallback("Failed to initialize CURL");
                }
                return;
            }
            g_isInitialized = true;
            LogMessage("DLL initialized successfully");

            // Create thread for API query
            CreateThread(NULL, 0, [](LPVOID) -> DWORD {
                Sleep(1000); // Give UI time to initialize
                queryapi();
                return 0;
                }, NULL, 0, NULL);
        }
    }

    __declspec(dllexport) void SetConsoleCallback(ConsoleCallbackType callback) {
        g_consoleCallback = callback;
    }

    __declspec(dllexport) void AttachMessage() {
        if (g_consoleCallback) {
            g_consoleCallback("Message attached successfully");
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_PROCESS_DETACH:
        if (g_isInitialized) {
            curl_global_cleanup();
            g_isInitialized = false;
        }
        break;
    }
    return TRUE;
}