#include "Config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <climits>
#include <cctype>

// statics
int Config::num_cpu = 0;
std::string Config::scheduler = "";
int Config::quantum_cycles = 0;
int Config::batch_process_freq = 0;
int Config::min_ins = 0;
int Config::max_ins = 0;
int Config::delay_per_exec = 0;
bool Config::loaded = false;

bool Config::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file '" << filename << "'\n";
        return false;
    }

    std::string line;
    int line_num = 1;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') { line_num++; continue; }

        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }

        if (tokens.empty()) { line_num++; continue; }

        //parse based on first token
        if (tokens[0] == "num-cpu") {
            if (tokens.size() != 2) goto invalid_line;
            try {
                int val = std::stoi(tokens[1]);
                if (val < 1 || val > 128) goto invalid_value;
                num_cpu = val;
            }
            catch (...) { goto invalid_value; }
        }
        else if (tokens[0] == "scheduler") {
            if (tokens.size() != 2) goto invalid_line;
            std::string val = tokens[1];
            if (val == "rr" || val == "\"rr\"") {
                scheduler = "rr";
            }
            else if (val == "fcfs" || val == "\"fcfs\"") {
                scheduler = "fcfs";
            }
            else {
                goto invalid_value;
            }
        }
        else if (tokens[0] == "quantum-cycles") {
            if (tokens.size() != 2) goto invalid_line;
            try {
                int val = std::stoi(tokens[1]);
                if (val < 1 || val > UINT32_MAX) goto invalid_value;                
                quantum_cycles = val;
            }
            catch (...) { goto invalid_value; }
        }
        else if (tokens[0] == "batch-process-freq") {
            if (tokens.size() != 2) goto invalid_line;
            try {
                int val = std::stoi(tokens[1]);
                if (val < 1 || val > UINT32_MAX) goto invalid_value;
                batch_process_freq = val;
            }
            catch (...) { goto invalid_value; }
        }
        else if (tokens[0] == "min-ins") {
            if (tokens.size() != 2) goto invalid_line;
            try {
                int val = std::stoi(tokens[1]);
                if (val < 1 || val > UINT32_MAX) goto invalid_value;
                min_ins = val;
            }
            catch (...) { goto invalid_value; }
        }
        else if (tokens[0] == "max-ins") {
            if (tokens.size() != 2) goto invalid_line;
            try {
                int val = std::stoi(tokens[1]);
                if (val < 1 || val > UINT32_MAX) goto invalid_value;
                max_ins = val;
            }
            catch (...) { goto invalid_value; }
        }
        else if (tokens[0] == "delay-per-exec") {
            if (tokens.size() != 2) goto invalid_line;
            try {
                int val = std::stoi(tokens[1]);
                if (val < 0 || val > UINT32_MAX) goto invalid_value;
                delay_per_exec = val;
            }
            catch (...) { goto invalid_value; }
        }
        else {
            std::cerr << "Unknown parameter at line " << line_num << ": " << tokens[0] << "\n";
            file.close();
            return false;
        }

        line_num++;
    }
    file.close();

    // Validate required fields
    if (num_cpu == 0 || scheduler.empty() || quantum_cycles == 0 ||
        batch_process_freq == 0 || min_ins == 0 || max_ins == 0) {
        std::cerr << "Missing or invalid required parameters.\n";
        return false;
    }

    if (min_ins > max_ins) {
        std::cerr << "min-ins cannot be greater than max-ins.\n";
        return false;
    }

    loaded = true;
    std::cout << "Config loaded successfully.\n";
    return true;

invalid_line:
    std::cerr << "Invalid format at line " << line_num << ": " << line << "\n";
    file.close();
    return false;

invalid_value:
    std::cerr << "Invalid value at line " << line_num << ": " << line << "\n";
    file.close();
    return false;
}

// Getters  
std::string Config::getScheduler() { return scheduler; }
int Config::getNumCpu() { return num_cpu; }
int Config::getQuantumCycles() { return quantum_cycles; }
int Config::getBatchProcessFreq() { return batch_process_freq; }
int Config::getMinIns() { return min_ins; }
int Config::getMaxIns() { return max_ins; }
int Config::getDelayPerExec() { return delay_per_exec; }

void Config::printSummary() {
    if (!loaded) return;
	std::cout << "===== Configuration Attributes =====\n";
    std::cout << "   num-cpu: " << num_cpu << "\n";
    std::cout << "   scheduler: " << scheduler << "\n";
    std::cout << "   quantum-cycles: " << quantum_cycles << "\n";
    std::cout << "   batch-process-freq: " << batch_process_freq << "\n";
    std::cout << "   min-ins: " << min_ins << "\n";
    std::cout << "   max-ins: " << max_ins << "\n";
    std::cout << "   delay-per-exec: " << delay_per_exec << "\n";
    std::cout << "====================================\n";

}