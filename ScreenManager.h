#pragma once
#include <string>
#include <vector>
#include "Process.h"


class ScreenManager {
public:
    static void listProcesses();
    static bool attachToProcess(const std::string& name);
    static void createAndAttach(const std::string& name); 
    static std::vector<Process>& getProcesses();
};