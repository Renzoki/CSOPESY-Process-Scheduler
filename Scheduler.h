#pragma once
#include <string>
#include <vector>
#include "Process.h"

struct CPUCore {
    int id;
    int currentProcessId = -1;
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
    static std::vector<Process>& getProcesses();
    static int getTotalTicks();
    static std::vector<CPUCore> cores;


private:
    static int nextProcessId;
    static std::string generateProcessName();
    static bool running;
    static int tickCounter;
    static std::vector<Process> global_processes;
    static int totalTicks;
    static int tickInterval;
    static std::vector<int> readyQueue;
    static std::vector<int> sleepingProcesses;
};