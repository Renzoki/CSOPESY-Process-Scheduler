#include "ScreenManager.h"
#include <random>
#include <iostream>
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
    for (int i = 0; i < count; ++i) {
        Instruction instr;
        instr.type = Instruction::PRINT;
        instr.args = { "\"Hello world from <name>!\"" };
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

    int minIns = 3;
    int maxIns = 8;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(minIns, maxIns);
    int numIns = dis(gen);
    auto instructions = generateDummyInstructions(numIns);
    Process newProc(name, instructions);
    global_processes.push_back(newProc);
    Process& procRef = global_processes.back();

    // mock data for now
    std::string cmd;
    while (true) {
        std::cout << procRef.getName() << ":> ";
        std::getline(std::cin, cmd);
        if (cmd == "exit") {
            break;
        }
        else if (cmd == "process-smi") {
            std::cout << "\nProcess name: " << name << "\n";
            std::cout << "ID: " << global_processes.size() << "\n";
            std::cout << "Logs:\n";
            std::cout << "(0/0/0 00:00:00AM) Core:0 \"Hello world from " << name << "!\"\n";
            std::cout << "Current instruction line: " << procRef.getCurrentLine() << "\n";
            std::cout << "Lines of code: " << procRef.getTotalLines() << "\n";
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
            std::cout << "ID: 1\n";
            std::cout << "Logs:\n";
            std::cout << "(0/0/0 00:00:00AM) Core:0 \"Hello world from " << it->getName() << "!\"\n";
            std::cout << "Current instruction line: " << it->getCurrentLine() << "\n";
            std::cout << "Lines of code: " << it->getTotalLines() << "\n";
        }
        else {
            std::cout << "Unknown command in screen.\n";
        }
    }
    return true;
}