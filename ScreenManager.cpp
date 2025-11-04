#include "ScreenManager.h"
#include <random>
#include "Config.h"
#include <iostream>
#include <iterator>
#include <algorithm>

std::vector<Process> global_processes;

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
            std::cout << p.getName() << "\n";
            found = true;
        }
    }
    if (!found) {
        std::cout << "No running processes.\n";
    }
}

std::vector<Instruction> generateDummyInstructions(int count) {
    std::vector<Instruction> ins;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> opDist(0, 5); // 0=PRINT, 1=DECLARE, etc

    for (int i = 0; i < count; ++i) {
        Instruction instr;
        int op = opDist(gen);
        if (op == 0) {
            instr.type = Instruction::PRINT;
            instr.args = { "\"Hello world from <name>!\"" };
        }
        else if (op == 1) {
            instr.type = Instruction::DECLARE;
            instr.args = { "x", "0" };
        }
        else if (op == 2) {
            instr.type = Instruction::ADD;
            instr.args = { "x", "5", "10" };
        }
        else if (op == 3) {
            instr.type = Instruction::SUBTRACT;
            instr.args = { "x", "x", "1" };
        }
        else if (op == 4) {
            instr.type = Instruction::SLEEP;
            instr.args = { "2" };
        }
        else {
            instr.type = Instruction::FOR;
            instr.args = { "2" }; // repeat 2 times
        }
        ins.push_back(instr);
    }
    return ins;
}

void ScreenManager::createAndAttach(const std::string& name) {
    // heck if process with same name already exists and is running
    auto it = std::find_if(global_processes.begin(), global_processes.end(),
        [&](const Process& p) {
            return p.getName() == name && !p.isFinished();
        });

    if (it != global_processes.end()) {
        std::cout << "Process " << name << " already exists.\n";
        return;
    }

    int minIns = Config::getMinIns();
    int maxIns = Config::getMaxIns();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(minIns, maxIns);
    int numIns = dis(gen);
    auto instructions = generateDummyInstructions(numIns);
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
            // TEMP: execute next instruction on every process-smi (for testing)
            if (!procRef.isFinished()) {
                procRef.executeNextInstruction();
            }
        }
        else {
            std::cout << "Unknown command in screen.\n";
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
        std::cout << it->getName() << ":> ";
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
            if (!it->isFinished()) {
                it->executeNextInstruction();
            }
        }
        else {
            std::cout << "Unknown command in screen.\n";
        }
    }
    return true;
}