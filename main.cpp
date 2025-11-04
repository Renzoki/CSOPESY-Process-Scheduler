#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <thread>
#ifdef _WIN32
#include <conio.h>
#endif
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
    int totalTicks = 0;

    std::cout << "CSOPESY MCO1 !!!\n";
    std::cout << ":> ";

    bool initialized = false;
    using Clock = std::chrono::steady_clock;
    auto lastTime = Clock::now();
    const int TICKS_PER_SECOND = 1;
    const auto TICK_DURATION = std::chrono::milliseconds(200 / TICKS_PER_SECOND);
    std::string inputBuffer = "";

    while (true) {
        // === CPU TICKS === working noq
        auto now = Clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime);
        int ticksElapsed = static_cast<int>(elapsed.count() / TICK_DURATION.count());

        if (ticksElapsed > 0) {
            for (int i = 0; i < ticksElapsed; ++i) {
                Scheduler::tick();
            }
            lastTime = now;
        }

        // user input (nesting hell)
#ifdef _WIN32
        if (_kbhit()) {
            char c = _getch();
            if (c == '\r' || c == '\n') {
                if (!inputBuffer.empty()) {
                    auto tokens = splitCommand(inputBuffer);
                    std::string cmd = tokens.empty() ? "" : tokens[0];

                    if (cmd == "exit") {

                        break;
                    }
                    else if (cmd == "initialize") {
                        if (tokens.size() != 1) {
                            std::cout << "\nUsage: initialize\n> ";
                        }
                        else if (!initialized) {
                            if (!Config::load("config.txt")) {
                                std::cout << "\nInitialization failed.\n> ";
                            }
                            else {
                                Config::printSummary();
                                Scheduler::initialize();
                                initialized = true;
                                std::cout << "> ";
                            }
                        }
                        else {
                            std::cout << "\nAlready initialized.\n> ";
                        }
                    }
                    else if (cmd == "screen") {
                        if (!initialized) {
                            std::cout << "\nError: Run 'initialize' first.\n> ";
                        }
                        else if (tokens.size() >= 2) {
                            if (tokens[1] == "-ls") {
                                ScreenManager::listProcesses();
                                std::cout << "> ";
                            }
                            else if (tokens[1] == "-s" && tokens.size() >= 3) {
                                ScreenManager::createAndAttach(tokens[2]);
                                std::cout << "> "; 

                            }
                            else if (tokens[1] == "-r" && tokens.size() >= 3) {
                                ScreenManager::attachToProcess(tokens[2]);
                                std::cout << "> "; 
                            }
                            else {
                                std::cout << "\nUsage: screen -ls | screen -s <name> | screen -r <name>\n> ";
                            }
                        }
                        else {
                            std::cout << "\nUsage: screen -ls | screen -s <name> | screen -r <name>\n> ";
                        }
                    }
                    else if (cmd == "scheduler-test") {
                        if (!initialized) {
                            std::cout << "\nError: Run 'initialize' first.\n> ";
                        }
                        else {
                            std::cout << "\nGenerating 5 dummy processes...\n";
                            Scheduler::generateBatch(5);
                            std::cout << "\nDone! :D\n> ";
                        }
                    }
                    else if (cmd == "scheduler-start") {
                        if (!initialized) {
                            std::cout << "\nError: Run 'initialize' first.\n> ";
                        }
                        else {
                            Scheduler::start();
                            std::cout << "> ";
                        }
                    }
                    else if (cmd == "scheduler-stop") {
                        Scheduler::stop();
                        std::cout << "> ";
                    }
                    else {
                        std::cout << "\nUnknown command: " << cmd << " >:( \n> ";
                    }
                    inputBuffer = "";
                }
                else {
                    std::cout << "\n> ";    
                }
            }
            else if (c == '\b') {
                if (!inputBuffer.empty()) {
                    inputBuffer.pop_back();
                    std::cout << "\b \b";
                }
            }
            else if (c >= 32 && c <= 126) { 
                inputBuffer += c;
                std::cout << c;
            }
        }
#endif

        // Small delay to reduce CPU usage (hopefully)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "\nThanks!\n";
    return 0;
}