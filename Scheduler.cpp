#include "Scheduler.h"
#include <iostream>
#include "Config.h"
#include "Process.h"
#include "ScreenManager.h"
#include <random>
#include <string>
#include <vector>
#include <algorithm>

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

    // Handle sleeping processes
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

    // Execute on all cores
    for (auto& core : cores) {
        if (core.currentProcessId != -1) {
            Process& proc = global_processes[core.currentProcessId];
            proc.executeNextInstruction(core.id);

            if (proc.isFinished()) {
                proc.setState(Process::FINISHED);
                core.currentProcessId = -1;
            }
            else if (proc.getState() == Process::SLEEPING) {
                // For RR: free core immediately
                if (Config::getScheduler() == "rr") {
                    sleepingProcesses.push_back(core.currentProcessId);
                    core.currentProcessId = -1;
                }
                // For FCFS: keep process on core (handled by sleep loop)
            }
            else {
                // RR preemption
                if (Config::getScheduler() == "rr") {
                    core.quantumRemaining--;
                    if (core.quantumRemaining <= 0) {
                        proc.setState(Process::READY);
                        // Avoid duplicate entries in readyQueue
                        if (std::find(readyQueue.begin(), readyQueue.end(), core.currentProcessId) == readyQueue.end()) {
                            readyQueue.push_back(core.currentProcessId);
                        }
                        core.currentProcessId = -1;
                    }
                }
                // FCFS: do nothing (runs to completion)
            }
        }
    }

    // Assign processes AFTER execution (critical for both schedulers)
    if (Config::getScheduler() == "fcfs") {
        // Multi-core FCFS: assign 1 process per free core
        for (auto& core : cores) {
            if (core.currentProcessId == -1 && !readyQueue.empty()) {
                int pid = readyQueue.front();
                readyQueue.erase(readyQueue.begin());
                core.currentProcessId = pid;
                global_processes[pid].setState(Process::RUNNING);
                global_processes[pid].setAssignedCoreId(core.id);
            }
        }
    }
    else { // Round Robin
        for (auto& core : cores) {
            if (core.currentProcessId == -1 && !readyQueue.empty()) {
                int pid = readyQueue.front();
                readyQueue.erase(readyQueue.begin());
                core.currentProcessId = pid;
                global_processes[pid].setState(Process::RUNNING);
                global_processes[pid].setAssignedCoreId(core.id);
                core.quantumRemaining = Config::getQuantumCycles();
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

std::vector<Instruction> generateDummyInstructions(int count) {
    std::vector<Instruction> ins;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> opDist(0, 5);

    for (int i = 0; i < count; ++i) {
        Instruction instr;
        int op = opDist(gen);

        switch (op) {
        case 0: // PRINT
            instr.type = Instruction::PRINT;
            instr.args = {};
            break;
        case 1: // DECLARE
            instr.type = Instruction::DECLARE;
            instr.args = { "x", std::to_string(gen() % 100) };
            break;
        case 2: // ADD
            instr.type = Instruction::ADD;
            instr.args = { "x", "5", "10" };
            break;
        case 3: // SUBTRACT
            instr.type = Instruction::SUBTRACT;
            instr.args = { "x", "x", "1" };
            break;
        case 4: // SLEEP
            instr.type = Instruction::SLEEP;
            instr.args = { std::to_string(gen() % 3 + 1) };
            break;
        case 5: // FOR
            instr.type = Instruction::FOR;
            instr.args = { "PRINT", std::to_string(gen() % 3 + 1) };
            break;
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