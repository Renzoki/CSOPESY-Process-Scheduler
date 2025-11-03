#include "ScreenManager.h"
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

    // new process
    Process newProc(name);
    global_processes.push_back(newProc);

    // mock data for now
    std::string cmd;
    while (true) {
        std::cout << name << ":> ";
        std::getline(std::cin, cmd);
        if (cmd == "exit") {
            break;
        }
        else if (cmd == "process-smi") {
            std::cout << "\nProcess name: " << name << "\n";
            std::cout << "ID: " << global_processes.size() << "\n";
            std::cout << "Logs:\n";
            std::cout << "(0/0/0 00:00:00AM) Core:0 \"Hello world from " << name << "!\"\n";
            std::cout << "Current instruction line: 0\n";
            std::cout << "Lines of code: 0\n";
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

    // Enter process screen loop
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
            std::cout << "Current instruction line: 0\n";
            std::cout << "Lines of code: 0\n";
        }
        else {
            std::cout << "Unknown command in screen.\n";
        }
    }
    return true;
}