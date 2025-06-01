#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <sys/time.h>
#include <time.h>
#include <map>
#include <iostream>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;
using std::map;

// Constructor
Process::Process(int pid) : pid_(pid) {
    user_ = LinuxParser::User(pid_);
    command_ = LinuxParser::Command(pid_);
    clock_gettime(CLOCK_MONOTONIC, &prev_time_);
    prev_jiffies_ = LinuxParser::ActiveJiffies(pid_);
    start_time_ = StartTime();
    cpu_utilization_ = CpuUtilization();
}

// Copy constructor
Process::Process(const Process& other) 
    : pid_(other.pid_),
      user_(other.user_),
      command_(other.command_),
      prev_jiffies_(other.prev_jiffies_),
      prev_time_(other.prev_time_),
      cpu_utilization_(other.cpu_utilization_) {
}

// Move constructor
Process::Process(Process&& other)
    : pid_(other.pid_),
      user_(std::move(other.user_)),
      command_(std::move(other.command_)),
      prev_jiffies_(other.prev_jiffies_),
      prev_time_(other.prev_time_),
      cpu_utilization_(other.cpu_utilization_) {
    // Reset the source object
    other.pid_ = 0;
    other.prev_jiffies_ = 0;
    other.prev_time_ = {};
    other.cpu_utilization_ = 0.0;
}

// Copy assignment operator
Process& Process::operator=(const Process& other) {
    if (this != &other) {
        pid_ = other.pid_;
        user_ = other.user_;
        command_ = other.command_;
        prev_jiffies_ = other.prev_jiffies_;
        prev_time_ = other.prev_time_;
        cpu_utilization_ = other.cpu_utilization_;
    }
    return *this;
}

// Move assignment operator
Process& Process::operator=(Process&& other) {
    if (this != &other) {
        pid_ = other.pid_;
        user_ = std::move(other.user_);
        command_ = std::move(other.command_);
        prev_jiffies_ = other.prev_jiffies_;
        prev_time_ = other.prev_time_;
        cpu_utilization_ = other.cpu_utilization_;
        
        // Reset the source object
        other.pid_ = 0;
        other.prev_jiffies_ = 0;
        other.prev_time_ = {};
        other.cpu_utilization_ = 0.0;
    }
    return *this;
}

// Member functions
int Process::Pid() const { 
    return pid_;
}

float Process::CpuUtilization() {
    long process_jiffies = LinuxParser::ActiveJiffies(pid_);
    
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    
    float elapsed_time = 0.0;
    if (prev_time_.tv_sec != 0) {  // Skip first run
        // Convert both timestamps to nanoseconds and subtract
        long long current_ns = (long long)current_time.tv_sec * 1000000000LL + current_time.tv_nsec;
        long long prev_ns = (long long)prev_time_.tv_sec * 1000000000LL + prev_time_.tv_nsec;
        elapsed_time = (current_ns - prev_ns) / 1000000000.0;  // Convert nanoseconds to seconds
    } else {
        prev_jiffies_ = process_jiffies;
        prev_time_ = current_time;
    }
    
    float cpu_utilization = 0.0;
    if (elapsed_time > 0.0) {
        cpu_utilization = (process_jiffies - prev_jiffies_) / 
                         (sysconf(_SC_CLK_TCK) * elapsed_time);
        prev_jiffies_ = process_jiffies;
        prev_time_ = current_time;
    }
    cpu_utilization_ = cpu_utilization;
    return cpu_utilization_;
}

string Process::Command() {
    return command_;
}

string Process::Ram() { 
    return LinuxParser::Ram(pid_);
}

string Process::User() { 
    return user_;
}

long int Process::UpTime() {
    return LinuxParser::UpTime(pid_) - StartTime().tv_sec;
}

// Calculates the start time using now - uptime
struct timespec Process::StartTime() {
    if (start_time_.tv_sec != 0) {
        return start_time_;
    }

    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    start_time_.tv_sec = LinuxParser::UpTime(pid_) - current_time.tv_sec ;
    return start_time_;
}

bool Process::operator<(Process& a) {
    return this->cpu_utilization_ > a.cpu_utilization_;
}
