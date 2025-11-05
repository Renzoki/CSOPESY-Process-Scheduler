#include "ScreenManager.h"
#include "Scheduler.h" 
#include <random>
#include "Config.h"
#include <iostream>
#include <iterator>
#include <chrono>
#include <fstream>
#include <iomanip> 
#include <algorithm>

static struct Initializer {
    Initializer() {
        Scheduler::getProcesses().reserve(10000);
    }
} init;


std::vector<Process>& ScreenManager::getProcesses() {
    return Scheduler::getProcesses();
}

void ScreenManager::listProcesses() {
    auto& procs = Scheduler::getProcesses();
    if (procs.empty()) {
        std::cout << "No processes at all.\n";
        return;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time);

    char dateStr[12];
    std::strftime(dateStr, sizeof(dateStr), "%m/%d/%Y", &tm);

    std::cout << "\nLast updated: " << dateStr << "\n\n";

    int totalCores = Config::getNumCpu();
    int usedCores = 0;
    for (const auto& core : Scheduler::cores) {
        if (core.currentProcessId != -1) {
            usedCores++;
        }
    }
    int cpuUtil = totalCores > 0 ? (usedCores * 100) / totalCores : 0;

    std::cout << "root:> screen -ls\n";
    std::cout << "CPU utilization: " << cpuUtil << "%\n";
    std::cout << "Cores used: " << usedCores << "\n";
    std::cout << "Cores available: " << (totalCores - usedCores) << "\n\n";

    std::cout << "----------------------------------------\n";
    std::cout << "Running processes:\n";

    bool foundRunning = false;
    for (const auto& p : procs) {
        if (!p.isFinished()) {
            auto procTime = std::chrono::system_clock::from_time_t(0); // Use process creation time? Or last log?
            std::tm procTm = tm;
            char timeStr[20];
            std::strftime(timeStr, sizeof(timeStr), "%m/%d/%Y %I:%M:%S%p", &procTm);

            std::cout << p.getName() << " (" << timeStr << ") "
                "Core: " << p.getAssignedCoreId()
                << "   " << p.getCurrentLine() << " / " << p.getTotalLines() << "\n";
            foundRunning = true;
        }
    }

    if (!foundRunning) {
        std::cout << "(none)\n";
    }

    std::cout << "\nFinished processes:\n";
    bool foundFinished = false;
    for (const auto& p : procs) {
        if (p.isFinished()) {
            // Same timestamp format
            char timeStr[20];
            std::strftime(timeStr, sizeof(timeStr), "%m/%d/%Y %I:%M:%S%p", &tm);

            std::cout << p.getName() << " (" << timeStr << ") "
                << "Finished   " << p.getCurrentLine() << " / " << p.getTotalLines() << "\n";
            foundFinished = true;
        }
    }

    if (!foundFinished) {
        std::cout << "(none)\n";
    }

    std::cout << "----------------------------------------\n";
}

void ScreenManager::createAndAttach(const std::string& name) {
    // heck if process with same name already exists and is running
    auto it = std::find_if(Scheduler::getProcesses().begin(), Scheduler::getProcesses().end(),
        [&](const Process& p) {
            return p.getName() == name && !p.isFinished();
        });

    if (it != Scheduler::getProcesses().end()) {
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
    Scheduler::getProcesses().push_back(newProc);
    Process& procRef = Scheduler::getProcesses().back();
    size_t procID = Scheduler::getProcesses().size(); //id

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
    auto& procs = Scheduler::getProcesses();
    Process* target = nullptr;
    size_t pid = 0;
    for (size_t i = 0; i < procs.size(); ++i) {
        if (procs[i].getName() == name && !procs[i].isFinished()) {
            target = &procs[i];
            pid = i;
            break;
        }
    }
    if (!target) {
        std::cout << "Process " << name << " not found.\n";
        return false;
    }

    auto it = std::find_if(Scheduler::getProcesses().begin(), Scheduler::getProcesses().end(),
        [&](const Process& p) {
            return p.getName() == name && !p.isFinished();
        });
    size_t procID = std::distance(Scheduler::getProcesses().begin(), it) + 1;

    if (it == Scheduler::getProcesses().end()) {
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

void ScreenManager::printUtilizationReport(bool toFile) {
    std::ofstream file;
    if (toFile) {
        file.open("csopesy-log.txt");
        if (!file.is_open()) {
            std::cout << "Error: Cannot open csopesy-log.txt for writing.\n";
            return;
        }
    }

    auto& procs = Scheduler::getProcesses();
    int totalCores = Config::getNumCpu();

    int running = 0;
    int finished = 0;
    for (const auto& p : procs) {
        if (p.isFinished()) finished++;
        else running++;
    }

    int usedCores = 0;
    for (const auto& core : Scheduler::getCores()) {
        if (core.currentProcessId != -1) {
            usedCores++;
        }
    }
    double utilization = (totalCores > 0) ? (100.0 * usedCores / totalCores) : 0.0;

    // Output stream
    std::ostream* outStream = toFile ? &file : &std::cout;

    (*outStream) << "===== CPU Utilization Report =====\n";
    (*outStream) << "Cores used: " << usedCores << " / " << totalCores << "\n";
    (*outStream) << std::fixed << std::setprecision(2)
        << "CPU Utilization: " << utilization << "%\n\n";

    (*outStream) << "Running Processes:\n";
    bool hasRunning = false;
    for (size_t i = 0; i < procs.size(); ++i) {
        const auto& p = procs[i];
        if (!p.isFinished()) {
            (*outStream) << "  " << p.getName() << " (ID " << i + 1 << ") - Line "
                << p.getCurrentLine() << " / " << p.getTotalLines() << "\n";
            hasRunning = true;
        }
    }
    if (!hasRunning) (*outStream) << "  None\n";

    (*outStream) << "\nFinished Processes:\n";
    bool hasFinished = false;
    for (size_t i = 0; i < procs.size(); ++i) {
        const auto& p = procs[i];
        if (p.isFinished()) {
            (*outStream) << "  " << p.getName() << " (ID " << i + 1 << ") - Line "
                << p.getCurrentLine() << " / " << p.getTotalLines() << "\n";
            hasFinished = true;
        }
    }
    if (!hasFinished) (*outStream) << "  None\n";

    (*outStream) << "==================================\n";

    if (toFile) {
        file.close();
        std::cout << "CPU utilization report saved to csopesy-log.txt\n";
    }
}