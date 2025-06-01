#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;
using std::map;

// TODO: Return the system's CPU
Processor& System::Cpu() { return cpu_; }

vector<Process>& System::Processes() { 
    vector<int> current_pids = LinuxParser::Pids();
    vector<Process> new_processes;
    map<int, Process> existing_processes;
    for (auto& process : processes_) {
        existing_processes[process.Pid()] = process;
    }

    for (int pid : current_pids) {
        auto it = existing_processes.find(pid);
        if (it != existing_processes.end()) {
            // TODO(mau): Process exists, check if it's one we've seen before
            new_processes.push_back(std::move(existing_processes[pid]));
        } else {
            new_processes.push_back(std::move(Process(pid)));
        }
    }
    processes_ = new_processes;
    std::sort(processes_.begin(), processes_.end());
    return processes_;
}

// TODO: Return the system's kernel identifier (string)
std::string System::Kernel() { 
    return kernel_;
 }

// TODO: Return the system's memory utilization
float System::MemoryUtilization() { 
    return LinuxParser::MemoryUtilization();
 }

// TODO: Return the operating system name
std::string System::OperatingSystem() { 
    return operating_system_;
 }

// TODO: Return the number of processes actively running on the system
int System::RunningProcesses() { 
    return LinuxParser::RunningProcesses();
 }

// TODO: Return the total number of processes on the system
int System::TotalProcesses() { 
    return LinuxParser::TotalProcesses();
 }

// TODO: Return the number of seconds since the system started running
long int System::UpTime() { 
    return LinuxParser::UpTime();
}
