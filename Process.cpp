#include "Process.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cctype>
#include <limits>
#include <iomanip>
#include <sstream>

Process::Process(const std::string& name, const std::vector<Instruction>& ins)
    : name(name), finished(false), instructions(ins), current_line(0), state(READY), sleepTicks(0) {
}

std::string Process::getName() const { return name; }
bool Process::isFinished() const { return finished; }
void Process::setFinished(bool f) { finished = f; }
size_t Process::getCurrentLine() const { return current_line; }
size_t Process::getTotalLines() const { return instructions.size(); }

uint16_t Process::getVariable(const std::string& var) const {
    auto it = variables.find(var);
    return (it != variables.end()) ? it->second : 0;
}

void Process::setVariable(const std::string& var, uint16_t value) {
    variables[var] = clampUint16(value);
}

const std::vector<std::string>& Process::getLogs() const {
    return logs;
}

// For manual/screen mode (no core ID)
void Process::addLog(const std::string& msg) {
    logs.push_back(msg);
    std::cout << "[" << name << "] " << msg << std::endl;
}

// For scheduler mode (with core ID and timestamp)
void Process::addLog(const std::string& msg, int coreId) {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%b/%d/%Y %I:%M:%S%p");
    std::string timestamp = oss.str();

    // Fix AM/PM case
    if (timestamp.size() >= 2) {
        if (timestamp.substr(timestamp.size() - 2) == "pm") {
            timestamp.replace(timestamp.size() - 2, 2, "PM");
        }
        else if (timestamp.substr(timestamp.size() - 2) == "am") {
            timestamp.replace(timestamp.size() - 2, 2, "AM");
        }
    }

    std::string logEntry = "(" + timestamp + ") Core:" + std::to_string(coreId) + " \"" + msg + "\"";
    logs.push_back(logEntry);
}

bool Process::isNumber(const std::string& s) const {
    if (s.empty()) return false;
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

uint16_t Process::getValue(const std::string& token) const {
    if (isNumber(token)) return static_cast<uint16_t>(std::stoi(token));
    auto it = variables.find(token);
    return (it != variables.end()) ? it->second : 0;
}

uint16_t Process::clampUint16(int64_t v) const {
    if (v < 0) return 0;
    if (v > std::numeric_limits<uint16_t>::max()) return std::numeric_limits<uint16_t>::max();
    return static_cast<uint16_t>(v);
}

std::string Process::instrTypeAsString(Instruction::Type type) const {
    switch (type) {
    case Instruction::PRINT: return "PRINT";
    case Instruction::DECLARE: return "DECLARE";
    case Instruction::ADD: return "ADD";
    case Instruction::SUBTRACT: return "SUBTRACT";
    case Instruction::SLEEP: return "SLEEP";
    case Instruction::FOR: return "FOR";
    default: return "UNKNOWN";
    }
}

// Public: Execute next instruction (called by scheduler or screen)
void Process::executeNextInstruction(int coreId, int nestedLevel) {
    if (finished || current_line >= instructions.size()) {
        finished = true;
        return;
    }
    executeInstruction(instructions[current_line], coreId, nestedLevel);
    current_line++;
    if (current_line >= instructions.size()) {
        finished = true;
    }
}

// Private: Execute a single instruction (handles all types)
void Process::executeInstruction(const Instruction& instr, int coreId, int nestedLevel) {
    if (nestedLevel > 3) {
        if (coreId >= 0) {
            addLog("Error: FOR loop nesting exceeded 3 levels!", coreId);
        }
        else {
            addLog("Error: FOR loop nesting exceeded 3 levels!");
        }
        return;
    }

    switch (instr.type) {
    case Instruction::PRINT: {
        std::string out;

        if (instr.args.empty()) {
            out = "Hello world from " + name + "!";
        }
        else {
            std::string msg = instr.args[0];

            if ((msg.size() >= 2) &&
                ((msg.front() == '"' && msg.back() == '"') ||
                    (msg.front() == '\'' && msg.back() == '\''))) {
                out = msg.substr(1, msg.size() - 2);
            }
            else if (variables.find(msg) != variables.end()) {
                out = std::to_string(variables[msg]);
            }
            else if (isNumber(msg)) {
                out = msg;
            }
            else {
                out = "Unknown symbol: " + msg;
            }
        }

        if (coreId >= 0) {
            addLog(out, coreId); // scheduler mode
        }
        else {
            addLog(out); // manual/screen mode
        }
        break;
    }

    case Instruction::DECLARE: {
        if (instr.args.size() < 2) break;
        long val = std::stol(instr.args[1]);
        setVariable(instr.args[0], clampUint16(val));
        break;
    }

    case Instruction::ADD: {
        if (instr.args.size() < 3) break;
        uint16_t a = getValue(instr.args[1]);
        uint16_t b = getValue(instr.args[2]);
        setVariable(instr.args[0], clampUint16(static_cast<int64_t>(a) + static_cast<int64_t>(b)));
        break;
    }

    case Instruction::SUBTRACT: {
        if (instr.args.size() < 3) break;
        uint16_t a = getValue(instr.args[1]);
        uint16_t b = getValue(instr.args[2]);
        setVariable(instr.args[0], clampUint16(static_cast<int64_t>(a) - static_cast<int64_t>(b)));
        break;
    }

    case Instruction::SLEEP: {
        if (instr.args.empty()) break;
        uint8_t ticks = static_cast<uint8_t>(std::stoi(instr.args[0]));

        if (coreId >= 0) {
            addLog("Sleeping for " + std::to_string(ticks) + " ticks...", coreId);
        }
        else {
            addLog("Sleeping for " + std::to_string(ticks) + " ticks...");
        }

        setSleepTicks(ticks);
        setState(SLEEPING);

        break;
    }

    case Instruction::FOR: {
        if (instr.args.size() < 2) break;

        std::string innerTypeStr = instr.args[0];
        int repeats = std::stoi(instr.args[1]);
        std::vector<std::string> innerArgs(instr.args.begin() + 2, instr.args.end());

        Instruction innerInstr;
        if (innerTypeStr == "PRINT") innerInstr.type = Instruction::PRINT;
        else if (innerTypeStr == "DECLARE") innerInstr.type = Instruction::DECLARE;
        else if (innerTypeStr == "ADD") innerInstr.type = Instruction::ADD;
        else if (innerTypeStr == "SUBTRACT") innerInstr.type = Instruction::SUBTRACT;
        else if (innerTypeStr == "SLEEP") innerInstr.type = Instruction::SLEEP;
        else if (innerTypeStr == "FOR") innerInstr.type = Instruction::FOR;
        else break;
        innerInstr.args = innerArgs;

        for (int i = 0; i < repeats; ++i) {
            std::string logMsg = "[FOR LOOP] Iteration " + std::to_string(i + 1) + "/" + std::to_string(repeats);
            if (coreId >= 0) {
                addLog(logMsg, coreId);
            }
            else {
                addLog(logMsg);
            }
            executeInstruction(innerInstr, coreId, nestedLevel + 1);
        }
        break;
    }

    default:
        if (coreId >= 0) {
            addLog("Unknown instruction type!", coreId);
        }
        else {
            addLog("Unknown instruction type!");
        }
        break;
    }
}

// Process state management
Process::State Process::getState() const {
    return state;
}

void Process::setState(Process::State s) {
    state = s;
}

int Process::getSleepTicks() const {
    return sleepTicks;
}

void Process::setSleepTicks(int ticks) {
    if (ticks < 0) ticks = 0;
    sleepTicks = ticks;
}