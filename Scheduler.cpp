#include "Scheduler.h"
#include <iostream>
#include "Config.h"
#include "Process.h"
#include "ScreenManager.h" // add process to global list
#include <random>
#include <string>
#include <vector>

int Scheduler::nextProcessId = 1;
int Scheduler::totalTicks = 0;
bool Scheduler::running = false;
std::vector<Process> Scheduler::global_processes;
int Scheduler::tickCounter = 0;
int Scheduler::tickInterval = 0;
std::vector<CPUCore> Scheduler::cores;
std::vector<int> Scheduler::readyQueue;
std::vector<int> Scheduler::sleepingProcesses;

std::vector<Process>& Scheduler::getProcesses() {
    return global_processes;
}

void Scheduler::initialize() {
    nextProcessId = 1;
    running = false;
    tickCounter = 0;
    tickInterval = 0;

    cores.clear();
    int numCpu = Config::getNumCpu();
    for (int i = 0; i < numCpu; ++i) {
        CPUCore core;
        core.id = i;
        cores.push_back(core);
    }

    readyQueue.clear();
    sleepingProcesses.clear();
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

int Scheduler::getTotalTicks() {
    return totalTicks;
}

void Scheduler::tick() {
    if (running) {
        tickCounter++;
        if (tickCounter >= tickInterval) {
            createDummyProcess();
            totalTicks++;
            tickCounter = 0;
        }
    }

    for (auto it = sleepingProcesses.begin(); it != sleepingProcesses.end();) {
        int pid = *it;
        Process& p = global_processes[pid];
        if (p.getSleepTicks() > 0) {
            p.setSleepTicks(p.getSleepTicks() - 1);
        }
        if (p.getSleepTicks() <= 0) {
            p.setState(Process::READY);
            readyQueue.push_back(pid);
            it = sleepingProcesses.erase(it);
        }
        else {
            ++it;
        }
    }

    for (auto& core : cores) {
        if (core.currentProcessId == -1 && !readyQueue.empty()) {
            core.currentProcessId = readyQueue.front();
            readyQueue.erase(readyQueue.begin());
            global_processes[core.currentProcessId].setState(Process::RUNNING);
            core.quantumRemaining = Config::getQuantumCycles();
        }
    }

    for (auto& core : cores) {
        if (core.currentProcessId != -1) {
            Process& proc = global_processes[core.currentProcessId];
            proc.executeNextInstruction(core.id);

            if (proc.isFinished()) {
                proc.setState(Process::FINISHED);
                core.currentProcessId = -1;
            }
            else if (proc.getState() == Process::SLEEPING) {
                sleepingProcesses.push_back(core.currentProcessId);
                core.currentProcessId = -1;
            }
            else {
                if (Config::getScheduler() == "rr") {
                    core.quantumRemaining--;
                    if (core.quantumRemaining <= 0) {
                        proc.setState(Process::READY);
                        readyQueue.push_back(core.currentProcessId);
                        core.currentProcessId = -1;
                    }
                }
            }
        }
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

//std::vector<Instruction> generateDummyInstructions(int count) { //test
//    std::vector<Instruction> ins;
//
//    Instruction d; d.type = Instruction::DECLARE; d.args = { "x", "5" }; ins.push_back(d);
//
//    Instruction a; a.type = Instruction::ADD; a.args = { "y", "x", "10" }; ins.push_back(a);
//
//    Instruction s; s.type = Instruction::SUBTRACT; s.args = { "z", "y", "3" }; ins.push_back(s);
//
//    Instruction px; px.type = Instruction::PRINT; px.args = { "x" }; ins.push_back(px);
//    Instruction py; py.type = Instruction::PRINT; py.args = { "y" }; ins.push_back(py);
//    Instruction pz; pz.type = Instruction::PRINT; pz.args = { "z" }; ins.push_back(pz);
//
//    // SLEEP
//    Instruction sl; sl.type = Instruction::SLEEP; sl.args = { "2" }; ins.push_back(sl);
//    Instruction p4; p4.type = Instruction::PRINT; p4.args = { "\"After sleep\"" }; ins.push_back(p4);
//    Instruction f; f.type = Instruction::FOR; f.args = { "PRINT", "2", "\"Loop\"" }; ins.push_back(f);
//
//    for (int i = 0; i < 100; ++i) {
//        Instruction p;
//        p.type = Instruction::PRINT;
//        p.args = {};
//        ins.push_back(p);
//    }
//
//    return ins; 
//}


std::vector<Instruction> generateDummyInstructions(int count) {
    std::vector<Instruction> ins;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> opDist(0, 5); // 0=PRINT, 1=DECLARE, ..., 5=FOR

    for (int i = 0; i < count; ++i) {
        Instruction instr;
        int op = opDist(gen);
        if (op == 0) {
            instr.type = Instruction::PRINT;
            instr.args = {}; 
        }
        else if (op == 1) {
            instr.type = Instruction::DECLARE;
            instr.args = {"x", "0"};
        }
        else if (op == 2) {
            instr.type = Instruction::ADD;
            instr.args = {"x", "5", "10"};
        }
        else if (op == 3) {
            instr.type = Instruction::SUBTRACT;
            instr.args = {"x", "x", "1"};
        }
        else if (op == 4) {
            instr.type = Instruction::SLEEP;
            instr.args = {"2"};
        }
        else {
            instr.type = Instruction::FOR;
            instr.args = { "PRINT", "3" }; 
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
    std::cout << "[DBG] Creating " << name << " with " << numIns << " instructions\n";

    auto instructions = generateDummyInstructions(numIns);

    // Add to Scheduler's storage
    global_processes.emplace_back(name, instructions);
    int processId = static_cast<int>(global_processes.size()) - 1;
    global_processes[processId].setState(Process::READY);
    readyQueue.push_back(processId);
}

void Scheduler::generateBatch(int count) {
    for (int i = 0; i < count; ++i) {
        createDummyProcess();
    }
}