#pragma once
#include <string>
#include <vector>
#include "Process.h"

class Scheduler {
public:
    static void initialize(); // for resetting state
    static void createDummyProcess();
    static void generateBatch(int count = 5);
    static void start();
    static void stop();
    static bool isRunning();
    static void tick();

private:
    static int nextProcessId;
    static std::string generateProcessName();
    static bool running;
    static int tickCounter;
    static int tickInterval;
};