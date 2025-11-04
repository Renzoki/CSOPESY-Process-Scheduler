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
int Scheduler::tickCounter = 0; //at this point i dont know which of these are used so im keeping them all
int Scheduler::tickInterval = 0; // will be set from config later
std::vector<CPUCore> Scheduler::cores;
std::vector<Process*> Scheduler::readyQueue;
std::vector<Process*> Scheduler::sleepingProcesses;

void Scheduler::initialize() {
    nextProcessId = 1;
    running = false;
    tickCounter = 0;
    tickInterval = 0;

    // Initialize CPU cores
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
        Process* p = *it;
        if (p->getSleepTicks() > 0) {
            p->setSleepTicks(p->getSleepTicks() - 1);
        }
        if (p->getSleepTicks() <= 0) { 
            p->setState(Process::READY);
            readyQueue.push_back(p);
            it = sleepingProcesses.erase(it);
        }
        else {
            ++it;
        }
    }

    for (auto& core : cores) {
        if (core.currentProcess == nullptr && !readyQueue.empty()) {
            core.currentProcess = readyQueue.front();
            readyQueue.erase(readyQueue.begin());
            core.currentProcess->setState(Process::RUNNING);
            core.quantumRemaining = Config::getQuantumCycles();
        }
    }

    for (auto& core : cores) {
        if (core.currentProcess != nullptr) {
            // do one instruction
            core.currentProcess->executeNextInstructionWithCore(core.id); 

            // check if finished
            if (core.currentProcess->isFinished()) {
                core.currentProcess->setState(Process::FINISHED);
                core.currentProcess = nullptr;
            }
            else {

                if (core.currentProcess->isFinished()) {
                    core.currentProcess->setState(Process::FINISHED);
                    core.currentProcess = nullptr;
                }
                else {
                    std::string schedType = Config::getScheduler();
                    if (schedType == "rr") {
                        core.quantumRemaining--;
                        if (core.quantumRemaining <= 0) {
                            core.currentProcess->setState(Process::READY);
                            readyQueue.push_back(core.currentProcess);
                            core.currentProcess = nullptr;
                        }
                    }
                }
            }
        }
    }

    //std::cout << "[DBG] Cores: " << cores.size()
    //    << ", Ready: " << readyQueue.size()
    //    << ", Sleeping: " << sleepingProcesses.size() << "\n";
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
    std::cout << "[DBG] Creating " << name << " with " << numIns << " instructions\n"; //debug
    auto instructions = generateDummyInstructions(numIns);
    Process newProc(name, instructions);
    ScreenManager::addProcess(newProc); // storage is in screenmanager
    Process& procRef = ScreenManager::getProcesses().back();
    procRef.setState(Process::READY);
    readyQueue.push_back(&procRef);
    //std::cout << "[debug] Added process to ready queue. Ready count: " << readyQueue.size() << "\n";
}

void Scheduler::generateBatch(int count) {
    for (int i = 0; i < count; ++i) {
        createDummyProcess();
    }
}

