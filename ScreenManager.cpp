#include "ScreenManager.h"
#include <random>
#include "Config.h"
#include <iostream>
#include <iterator>
#include <algorithm>
#include <sstream>

std::vector<Process> global_processes;

Process* getProcessByName(const std::string& name) {
    for (auto& p : global_processes) {
        if (p.getName() == name && !p.isFinished()) {
            return &p;
        }
    }
    return nullptr;
}

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
    // Check if process with same name already exists and is running
    auto it = std::find_if(global_processes.begin(), global_processes.end(),
        [&](const Process& p) {
            return p.getName() == name && !p.isFinished();
        });

    if (it != global_processes.end()) {
        std::cout << "Process " << name << " already exists.\n";
        return;
    }

    // int minIns = Config::getMinIns();
    std::vector<Instruction> instructions;
    /*Instruction printInstr;
    printInstr.type = Instruction::PRINT;
    printInstr.args = { "\"Hello world from <name>!\"" };
    instructions.push_back(printInstr);
    */
    Process newProc(name, instructions);
    global_processes.push_back(newProc);
    Process& procRef = global_processes.back();
    size_t procID = global_processes.size(); //id

    std::string cmd;
    while (true) {
        std::cout << procRef.getName() << ":> ";
        std::getline(std::cin, cmd);
        if (cmd.empty()) continue;

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
        else { // Manual instruction parser - robust version
            // trim leading spaces
            auto ltrim = [](std::string& s) {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
                };
            auto rtrim = [](std::string& s) {
                s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
                };
            auto trim = [&](std::string& s) { ltrim(s); rtrim(s); };

            std::string line = cmd;
            trim(line);
            if (line.empty()) continue;

            size_t pos = 0;
            while (pos < line.size() && !std::isspace((unsigned char)line[pos]) && line[pos] != '(') ++pos;
            std::string cmdName = line.substr(0, pos);

            std::string cmdUpper = cmdName;
            for (auto& c : cmdUpper) c = (char)std::toupper((unsigned char)c);

            Instruction instr;

            if (cmdUpper == "PRINT") {
                size_t start = line.find('(');
                size_t end = line.rfind(')');
                std::string content;

                if (start != std::string::npos && end != std::string::npos && end > start) {
                    content = line.substr(start + 1, end - start - 1);
                    trim(content);
                }
                else {
                    std::string rest = line.substr(pos);
                    trim(rest);
                    content = rest;
                }

                if (!content.empty() && (content.front() == '"' || content.front() == '\'')) {
                    // keep quotes so Process.cpp can handle them
                    instr.type = Instruction::PRINT;
                    instr.args = { content };
                    procRef.executeInstruction(instr);
                    procRef.addLog("Executed PRINT literal (quoted): " + content);
                }

                else {
                    trim(content);
                    if (content.empty()) {
                        instr.type = Instruction::PRINT;
                        instr.args = {};
                        procRef.executeInstruction(instr);
                        procRef.addLog("Executed PRINT (default)");
                    }
                    else {
                        std::vector<std::string> args;
                        std::string tmp = content;
                        std::replace(tmp.begin(), tmp.end(), ',', ' ');
                        std::istringstream issArgs(tmp);
                        std::string tok;
                        while (issArgs >> tok) args.push_back(tok);
                        instr.type = Instruction::PRINT;
                        instr.args = args;
                        procRef.executeInstruction(instr);
                        procRef.addLog("Executed PRINT with args");
                    }
                }
            }
            else if (cmdUpper == "DECLARE" || cmdUpper == "ADD" || cmdUpper == "SUBTRACT" ||
                cmdUpper == "SLEEP" || cmdUpper == "FOR") {
                std::string argstr;
                size_t pOpen = line.find('(');
                size_t pClose = line.rfind(')');
                if (pOpen != std::string::npos && pClose != std::string::npos && pClose > pOpen) {
                    argstr = line.substr(pOpen + 1, pClose - pOpen - 1);
                }
                else {
                    std::string rest = line.substr(pos);
                    trim(rest);
                    argstr = rest;
                }

                std::vector<std::string> args;
                std::string tmp = argstr;
                std::replace(tmp.begin(), tmp.end(), ',', ' ');
                std::istringstream iss2(tmp);
                std::string a;
                while (iss2 >> a) args.push_back(a);

                if (cmdUpper == "DECLARE") instr.type = Instruction::DECLARE;
                else if (cmdUpper == "ADD") instr.type = Instruction::ADD;
                else if (cmdUpper == "SUBTRACT") instr.type = Instruction::SUBTRACT;
                else if (cmdUpper == "SLEEP") instr.type = Instruction::SLEEP;
                else if (cmdUpper == "FOR") instr.type = Instruction::FOR;

                instr.args = args;
                procRef.executeInstruction(instr);
                procRef.addLog("Executed " + cmdUpper);
            }
            else {
                std::cout << "Unknown instruction: " << cmdName << "\n";
            }
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
