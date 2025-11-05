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

    std::cout << R"(
   _____   _____   ____   _____   ______    _____  __   __
  / ____| / ____| / __ \ |  __ \ |  ____|  / ____| \ \ / /
 | |     | (___  | |  | || |__) || |___   | (___    \ V / 
 | |      \___ \ | |  | ||  ___/ |  ___|   \___ \    | |
 | |____  ____) || |__| || |     | |____   ____) |   | |        
  \_____||_____/  \____/ |_|     |______| |_____/    |_|      
 
    )" << std::endl;
    std::cout << "-----------------------------------------------------\n";
    std::cout << "Welcome to CSOPESY Emulator!\n\n";
    std::cout << "Developers:\n";
    std::cout << "Fourrier, Lara\n";
    std::cout << "Laxa, Joshua\n";
    std::cout << "Tabuzo, Vincent\n\n";
    std::cout << "Last updated: 11-5-2025\n";
    std::cout << "-----------------------------------------------------\n\n";
 


    while (true) {

        Scheduler::tick(); // ticks!



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

                    //ScreenManager::listProcesses();

                    ScreenManager sm;

                    sm.printUtilizationReport(false);

                }



                else if (tokens[1] == "-util") {

                    if (!initialized) {

                        std::cout << "Error: Run 'initialize' first.\n";

                        continue;

                    }

                    ScreenManager sm;

                    sm.printUtilizationReport(true);

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

        else if (cmd == "scheduler-start") {

            if (!initialized) {

                std::cout << "Error: Run 'initialize' first.\n";

                continue;

            }

            Scheduler::start();

        }

        else if (cmd == "scheduler-stop") {

            Scheduler::stop();

        }

        else {

            std::cout << "Unknown command: " << cmd << " >:( \n";

        }



    }



    std::cout << "Thanks!\n";

    return 0;

}
