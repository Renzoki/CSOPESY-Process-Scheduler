#pragma once
#include <string>

class Process {
public:
    Process(const std::string& name);
    std::string getName() const;
    bool isFinished() const;
    void setFinished(bool f);

private:
    std::string name;
    bool finished;
};