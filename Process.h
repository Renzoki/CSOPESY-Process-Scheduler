#pragma once
#include <string>
#include <vector>
#include <map>

struct Instruction {
    enum Type { PRINT, DECLARE, ADD, SUBTRACT, SLEEP, FOR };
    Type type = Type::PRINT;
    std::vector<std::string> args;
};

class Process {
public:
    Process(const std::string& name, const std::vector<Instruction>& instructions);
    std::string getName() const;
    bool isFinished() const;
    void setFinished(bool f);
    size_t getCurrentLine() const;
    size_t getTotalLines() const;
    void advanceLine();
    uint16_t getVariable(const std::string& name) const;
    void setVariable(const std::string& name, uint16_t value); 
    const std::vector<std::string>& getLogs() const; 
    void addLog(const std::string& msg); 
    void executeNextInstruction();

private:
    std::string name;
    bool finished;
    std::vector<Instruction> instructions;
    size_t current_line;
    std::map<std::string, uint16_t> variables;
    std::vector<std::string> logs;
};