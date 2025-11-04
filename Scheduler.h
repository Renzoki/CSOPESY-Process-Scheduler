#pragma once
#include <string>
#include <vector>
#include "Process.h"

class Scheduler {
public:
    static void initialize(); // for resetting state
    static void createDummyProcess();
    static void generateBatch(int count = 5);

private:
    static int nextProcessId;
    static std::string generateProcessName();
};