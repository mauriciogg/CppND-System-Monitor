#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"

using std::string;
using std::to_string;
using std::vector;

// TODO: Return this process's ID
int Process::Pid() { 
    return pid_;
 }

// TODO: Return this process's CPU utilization
float Process::CpuUtilization() {
    float cpu_utilization = 0.0;
    float total_time = LinuxParser::ActiveJiffies(pid_);
    float seconds = LinuxParser::UpTime(pid_);
    cpu_utilization = (total_time / seconds) / 100;
    return cpu_utilization;
}
// TODO: Return the command that generated this process
string Process::Command() {
    return command_;
 }

// TODO: Return this process's memory utilization
string Process::Ram() { 
    LinuxParser::Ram(pid_);
 }

// TODO: Return the user (name) that generated this process
string Process::User() { 
    return user_;
}

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() {
    return LinuxParser::UpTime(pid_);
 }

// TODO: Overload the "less than" comparison operator for Process objects
// REMOVE: [[maybe_unused]] once you define the function
bool Process::operator<(Process const& a[[maybe_unused]]) const {
    // TODO: sort by PID bc its the easiest, but we should add different
    // sorting methods (mem, cpu, pid, name ...)
    return pid_ < a.pid_;
}