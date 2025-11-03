#include "Process.h"

Process::Process(const std::string& name) : name(name), finished(false) {}

std::string Process::getName() const { return name; }
bool Process::isFinished() const { return finished; }
void Process::setFinished(bool f) { finished = f; }