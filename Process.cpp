#include "Process.h"

Process::Process(const std::string& name, const std::vector<Instruction>& ins)
    : name(name), finished(false), instructions(ins), current_line(0) {
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

uint16_t Process::getVariable(const std::string& name) const {
    auto it = variables.find(name);
    return (it != variables.end()) ? it->second : 0;
}

void Process::setVariable(const std::string& name, uint16_t value) {
    if (value > 65535) value = 65535;
    variables[name] = value;
}