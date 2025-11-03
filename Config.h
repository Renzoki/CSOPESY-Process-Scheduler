#pragma once
#include <string>

class Config {
public:
    static bool load(const std::string& filename);

    static int getNumCpu();
    static std::string getScheduler();
    static int getQuantumCycles();
    static int getBatchProcessFreq();
    static int getMinIns();
    static int getMaxIns();
    static int getDelayPerExec();
    static void printSummary();

private:
    static int num_cpu;
    static std::string scheduler;
    static int quantum_cycles;
    static int batch_process_freq;
    static int min_ins;
    static int max_ins;
    static int delay_per_exec;
    static bool loaded;
};