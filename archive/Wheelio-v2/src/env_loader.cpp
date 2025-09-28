#include "config.h"
#include <fstream>
#include <string>
#include <map>

// Define the env variable
std::map<std::string, std::string> env;

// Function to parse .env file
std::map<std::string, std::string> parseEnvFile(const std::string &filePath) {
    std::map<std::string, std::string> envVariables;
    std::ifstream envFile(filePath);
    if (envFile.is_open()) {
        std::string line;
        while (std::getline(envFile, line)) {
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);
                envVariables[key] = value;
            }
        }
        envFile.close();
    }
    return envVariables;
}

// Load the .env file
void loadEnv() {
    env = parseEnvFile(".env");
}