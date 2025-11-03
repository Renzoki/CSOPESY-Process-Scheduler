#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "Config.h"

std::vector<std::string> splitCommand(const std::string& cmd) {
    std::istringstream iss(cmd);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

int main() {
    std::string input;
    bool initialized = false;

    std::cout << "CSOPESY MCO1 !!!\n";
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input.empty()) continue;

        auto tokens = splitCommand(input);
        std::string cmd = tokens[0];

        if (cmd == "exit") {
            break;
        }
        else if (cmd == "initialize") {
            if (tokens.size() != 1) {
                std::cout << "Usage: initialize\n";
                continue;
            }
            
            if (!Config::load("config.txt")) {
                std::cout << "Initialization failed.\n";
                continue;
            }
            Config::printSummary();
            initialized = true;
        }
        else if (cmd == "screen") {
            if (!initialized) {
                std::cout << "Error: Run 'initialize' first.\n";
                continue;
            }
            if (tokens.size() >= 2 && tokens[1] == "-ls") {
                std::cout << "No running processes.\n";
            }
            else {
                std::cout << "Usage: screen -ls\n";
            }
        }
        else if (cmd == "scheduler-start" || cmd == "scheduler-stop" ||
            cmd == "scheduler-test" || cmd == "report-util") {
            if (!initialized) {
                std::cout << "Error: Run 'initialize' first.\n";
                continue;
            }
            std::cout << "Command not implemented yet :(\n";
        }
        else {
            std::cout << "Unknown command: " << cmd << " >:( \n";
        }
    }

    std::cout << "Thanks!\n";
    return 0;
}