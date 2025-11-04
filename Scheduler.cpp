#include "Scheduler.h"
#include <iostream>
#include "Config.h"
#include "ScreenManager.h" // add process to global list
#include <random>
#include <string>

int Scheduler::nextProcessId = 1;
bool Scheduler::running = false;
int Scheduler::tickCounter = 0;
int Scheduler::tickInterval = 1; // will be set from config later

void Scheduler::initialize() {
    nextProcessId = 1;
}

void Scheduler::start() {
    running = true;
    tickInterval = Config::getBatchProcessFreq();
    std::cout << "Scheduler started. Generating a process every "
        << tickInterval << " ticks.\n";
}

void Scheduler::stop() {
    running = false;
    std::cout << "Scheduler stopped.\n";
}

bool Scheduler::isRunning() {
    return running;
}

void Scheduler::tick() {
    if (!running) return;
    tickCounter++;
    if (tickCounter >= tickInterval) {
        createDummyProcess();
        tickCounter = 0;
    }
}

std::string Scheduler::generateProcessName() {
    std::string name = "p";
    if (nextProcessId < 10) {
        name += "0";
    }
    name += std::to_string(nextProcessId++);
    return name;
}

std::vector<Instruction> generateDummyInstructions(int count) {
    std::vector<Instruction> ins;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> opDist(0, 5);

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
            instr.args = { "2" };
        }
        ins.push_back(instr);
    }
    return ins;
}

void Scheduler::createDummyProcess() {
    std::string name = generateProcessName();
    int minIns = Config::getMinIns();
    int maxIns = Config::getMaxIns();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(minIns, maxIns);
    int numIns = dis(gen);
    auto instructions = generateDummyInstructions(numIns);
    Process newProc(name, instructions);
    ScreenManager::addProcess(newProc); // storage is in screenmanager
}

void Scheduler::generateBatch(int count) {
    for (int i = 0; i < count; ++i) {
        createDummyProcess();
    }
}

