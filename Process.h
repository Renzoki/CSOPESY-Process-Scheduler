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

    uint16_t getVariable(const std::string& name) const;
    void setVariable(const std::string& name, uint16_t value);

    const std::vector<std::string>& getLogs() const;
    void addLog(const std::string& msg);              // for screen/manual
    void addLog(const std::string& msg, int coreId);  // for scheduler
    int getAssignedCoreId() const { return assignedCoreId; }
    void setAssignedCoreId(int id) { assignedCoreId = id; }

    void executeNextInstruction(int coreId = -1, int nestedLevel = 0);
    void executeInstruction(const Instruction& instr, int coreId, int nestedLevel);

    enum State { READY, RUNNING, SLEEPING, FINISHED };
    State getState() const;
    void setState(State s);
    int getSleepTicks() const;
    void setSleepTicks(int ticks);

private:
    std::string name;
    bool finished;
    std::vector<Instruction> instructions;
    size_t current_line;
    std::map<std::string, uint16_t> variables;
    std::vector<std::string> logs;
    int assignedCoreId = -1;

    State state;
    int sleepTicks;

    bool isNumber(const std::string& s) const;
    uint16_t getValue(const std::string& token) const;
    uint16_t clampUint16(int64_t v) const;
    std::string instrTypeAsString(Instruction::Type type) const;
};