#include "Process.h"
#include <iostream>
#include <chrono>
#include <ctime>
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

void Process::advanceLine() {
    if (current_line < instructions.size()) {
        current_line++;
    }
}

void Process::executeNextInstructionWithCore(int coreId) {
    if (finished || current_line >= instructions.size()) {
        finished = true;
        return;
    }

    const Instruction& instr = instructions[current_line];

    if (instr.type == Instruction::PRINT) {
        std::string msg = "Hello world from " + name + "!";
        addLog(msg, coreId); 
    }

    advanceLine();

    if (current_line >= instructions.size()) {
        finished = true;
    }
}

uint16_t Process::getVariable(const std::string& name) const {
    auto it = variables.find(name);
    return (it != variables.end()) ? it->second : 0;
}

void Process::setVariable(const std::string& name, uint16_t value) {
    if (value > 65535) value = 65535;
    variables[name] = value;
}

const std::vector<std::string>& Process::getLogs() const {
    return logs;
}

void Process::addLog(const std::string& msg, int coreId) {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time); 

    std::ostringstream oss;
    oss << std::put_time(&tm, "%b/%d/%Y %I:%M:%S%p");
    std::string timestamp = oss.str();

    if (timestamp.back() == 'p') {
        timestamp.replace(timestamp.size() - 2, 2, "PM");
    }
    else {
        timestamp.replace(timestamp.size() - 2, 2, "AM");
    }

    std::string logEntry = "(" + timestamp + ") Core:" + std::to_string(coreId) + " \"" + msg + "\"";
    logs.push_back(logEntry);
}

void Process::executeNextInstruction() {
    if (finished || current_line >= instructions.size()) {
        finished = true;
        return;
    }

    const Instruction& instr = instructions[current_line];

    if (instr.type == Instruction::PRINT) { //unfinished
        std::string msg = "(0/0/0 00:00:00AM) Core:0 \"Hello world from " + name + "!\"";

    }
    advanceLine();

    if (current_line >= instructions.size()) {
        finished = true;
    }
}

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