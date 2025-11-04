#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "Config.h"
#include "ScreenManager.h"
#include "Scheduler.h"

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
            if (tokens.size() >= 2) {
                if (tokens[1] == "-ls") {
                    ScreenManager::listProcesses();
                }
                else if (tokens[1] == "-s" && tokens.size() >= 3) {
                    ScreenManager::createAndAttach(tokens[2]);
                }
                else if (tokens[1] == "-r" && tokens.size() >= 3) {
                    ScreenManager::attachToProcess(tokens[2]); 
                }
                else {
                    std::cout << "Usage: screen -ls | screen -s <name> | screen -r <name>\n";
                }
            }
            else {
                std::cout << "Usage: screen -ls | screen -s <name> | screen -r <name>\n";
            }
        }
        else if (cmd == "scheduler-test") {
            if (!initialized) {
                std::cout << "Error: Run 'initialize' first.\n";
                continue;
            }
            std::cout << "Generating 5 dummy processes...\n";
            Scheduler::generateBatch(5);
            std::cout << "Done! :D\n";
        }
        else {
            std::cout << "Unknown command: " << cmd << " >:( \n";
        }
    }

    std::cout << "Thanks!\n";
    return 0;
}