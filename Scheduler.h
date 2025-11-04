#pragma once
#include <string>
#include <vector>
#include "Process.h"

struct CPUCore {
    int id;
    class Process* currentProcess = nullptr;
    int quantumRemaining = 0;
};

class Scheduler {
public:
    static void initialize(); // for resetting state
    static void createDummyProcess();
    static void generateBatch(int count = 5);
    static void start();
    static void stop();
    static bool isRunning();
    static void tick();
    static int getTotalTicks();


private:
    static int nextProcessId;
    static std::string generateProcessName();
    static bool running;
    static int tickCounter;
    static int totalTicks;
    static int tickInterval;
    static std::vector<CPUCore> cores;
    static std::vector<class Process*> readyQueue;
    static std::vector<class Process*> sleepingProcesses;
};