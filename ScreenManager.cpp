#include "ScreenManager.h"
#include "Scheduler.h" 
#include <random>
#include "Config.h"
#include <iostream>
#include <iterator>
#include <algorithm>

std::vector<Process> global_processes;
static struct Initializer {
    Initializer() {
        global_processes.reserve(10000); 
    }
} init;


std::vector<Process>& ScreenManager::getProcesses() {
    return global_processes;
}

void ScreenManager::addProcess(const Process& p) {
    global_processes.push_back(p);
}

void ScreenManager::listProcesses() {
    bool found = false;
    for (const auto& p : global_processes) {
        if (!p.isFinished()) {
            std::cout << "\n" << p.getName() << " (line " << p.getCurrentLine() << ")\n";
            found = true;
        }
    }
    if (!found) {
        std::cout << "No running processes.\n";
    }
    std::cout << "\n===============\n";
}

void ScreenManager::createAndAttach(const std::string& name) {
    // heck if process with same name already exists and is running
    auto it = std::find_if(global_processes.begin(), global_processes.end(),
        [&](const Process& p) {
            return p.getName() == name && !p.isFinished();
        });

    if (it != global_processes.end()) {
        std::cout << "\nProcess " << name << " already exists.\n";
        return;
    }

    int minIns = Config::getMinIns();
    std::vector<Instruction> instructions;
    Instruction printInstr;
    printInstr.type = Instruction::PRINT;
    printInstr.args = { "\"\nHello world from <name>!\"" };
    instructions.push_back(printInstr);
    Process newProc(name, instructions);
    global_processes.push_back(newProc);
    Process& procRef = global_processes.back();
    size_t procID = global_processes.size(); //id

    std::string cmd;
    while (true) {
        std::cout << procRef.getName() << ":> ";
        std::getline(std::cin, cmd);
        if (cmd == "exit") {
            break;
        }
        else if (cmd == "process-smi") {
            std::cout << "\nProcess name: " << name << "\n";
            std::cout << "ID: " << procID << "\n";
            std::cout << "Logs:\n";

            const auto& logs = procRef.getLogs();
            if (logs.empty()) {
                std::cout << "(No logs yet)\n";
            }
            else {
                for (const auto& log : logs) {
                    std::cout << log << "\n";
                }
            }

            std::cout << "Current instruction line: " << procRef.getCurrentLine() << "\n";
            std::cout << "Lines of code: " << procRef.getTotalLines() << "\n";
        }
        else {
            std::cout << "\nUnknown command in screen.\n";
        }
    }
}

bool ScreenManager::attachToProcess(const std::string& name) {
    auto it = std::find_if(global_processes.begin(), global_processes.end(),
        [&](const Process& p) {
            return p.getName() == name && !p.isFinished();
        });
    size_t procID = std::distance(global_processes.begin(), it) + 1;

    if (it == global_processes.end()) {
        std::cout << "Process " << name << " not found.\n";
        return false;
    }

    std::string cmd;
    while (true) {
        std::cout << "\n" << it->getName() << ":> ";
        std::getline(std::cin, cmd);
        if (cmd == "exit") {
            break;
        }
        else if (cmd == "process-smi") {
            std::cout << "\nProcess name: " << it->getName() << "\n";
            std::cout << "ID: " << procID << "\n";
            std::cout << "Logs:\n";

            const auto& logs = it->getLogs(); 
            if (logs.empty()) {
                std::cout << "(No logs yet)\n";
            }
            else {
                for (const auto& log : logs) {
                    std::cout << log << "\n";
                }
            }

            std::cout << "Current instruction line: " << it->getCurrentLine() << "\n";
            std::cout << "Lines of code: " << it->getTotalLines() << "\n";

        }
        else {
            std::cout << "\nUnknown command in screen.\n";
        }
    }
    return true;
}