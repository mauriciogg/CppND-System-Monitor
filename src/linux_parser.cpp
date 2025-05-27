#include "linux_parser.h"

#include <cassert>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <tuple>
#include <unistd.h>
#include <vector>

using std::tuple;
using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
	vector<int> pids;
	const std::filesystem::path proc_dir_path{kProcDirectory};
	for (auto const& dir_entry : std::filesystem::directory_iterator{proc_dir_path}) {
		if (!dir_entry.is_directory()) {
			continue;
		}

		string filename = dir_entry.path().filename().string();
		if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        	int pid = stoi(filename);
        	pids.push_back(pid);
		}
	}
	return pids;
}

// TODO(mgg): replace with std::getline() and support a arbitrary number of splits
tuple<string, string> split(const string& s, const string delimiter) {
		auto pos = s.find(delimiter);

		// TODO(mgg): handle gracefullly
		assert(pos != string::npos);

		auto key = s.substr(0, pos);
		auto value = s.substr(pos + 1, s.length());
		return std::make_tuple(key, value);
}


// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
   // Using same the simplest available option: MemTotal - MemFree / MemTotal.
	// NOTE: this is a top limit since reclaimable memory is not accounted for here.
	// MemTotal:       49334576 kB
  // MemFree:        47392868 kB
  double memtotal;
	double memfree;

  std::ifstream stream(kProcDirectory + kMeminfoFilename);
	string delimiter(":");
  if (stream.is_open()) {
		string memtotal_line;
		string memfree_line;

    std::getline(stream, memtotal_line);
		tuple pts = split(memtotal_line, ":");
		auto key = std::get<0>(pts);
		auto value = std::get<1>(pts);
		assert(key == "MemTotal");
		memtotal = stoi(std::get<1>(split(value, " ")));
	
    std::getline(stream, memfree_line);
		pts = split(memfree_line, ":");
		key = std::get<0>(pts);
		value = std::get<1>(pts);
		assert(key == "MemFree");
		// Assume kb
		memfree = stoi(std::get<1>(split(value, " ")));
  }
	return memfree / memtotal;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
	// we get this from /proc/uptime
  std::ifstream stream(kProcDirectory + kUptimeFilename);
	string delimiter(" ");
  if (stream.is_open()) {
		string uptime_line;
    std::getline(stream, uptime_line);
		tuple pts = split(uptime_line, " ");
		return std::stof(std::get<0>(pts));
	}
	return 0; 
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  // we get this from /proc/stat
  // the first line contains the time spent (Jiffies- Hz)
  // we need to parse it and return the sum of the values
  // e.g 
  // cpu  112656 112656 112656 112656 112656 112656 112656 112656 112656 112656
  std::ifstream stream(kProcDirectory + kStatFilename);
  string line;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    string cpu;
    long total_jiffies = 0;
    long value;
    
    // skip cpu field
    linestream >> cpu;
    
    while (linestream >> value) {
      total_jiffies += value;
    }
    return total_jiffies;
  }
  return 0;
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) { 
  // /proc/<pid>/stat
  // PID (<p_name>) R 255664 256578 255664 34818 256578 4194304 104 0 0 0 0 0 0 0 20 0 1 0 13823196 3346432 265 18446744073709551615 94358669713408 94358669730993 140725682901616 0 0 0 0 0 0 0 0 0 17 0 0 0 0 0 0 94358669744784 94358669746280 94359282098176 140725682906714 140725682906734 140725682906734 140725682909163 0
  // utime (14) - user code time
  // stime (15) - kernel code time
  // cutime (16) - user code time for proc children
  // cstime (17) - kernel code time for proc children
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  string line;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    string value;
    
    for (int i = 0; i < 13; i++) {
      linestream >> value;
    }
    
    long utime, stime, cutime, cstime;
    linestream >> utime >> stime >> cutime >> cstime;
    return utime + stime + cutime + cstime;
  }
  return 0;
}

/* /proc/stat 
 * cpu  102159 240 258017 220437998 26027 0 13273 0 0 0
 * ...
*/

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  // Active jiffies = user + nice + system + irq + softirq + stel + guest + guest_nice
  // Active jiffies are the sum of:
  std::ifstream stream(kProcDirectory + kStatFilename);
  string line;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    string cpu;
    long value;
    long active_jiffies = 0;
    
    linestream >> cpu;
    
    // Read and sum the active jiffies
    // Fields are: user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice
    for (int i = 0; i < 10; i++) {
      linestream >> value;
      // Skip idle (i=3) and iowait (i=4) as they are not active time
      if (i != 3 && i != 4) {
        active_jiffies += value;
      }
    }
    return active_jiffies;
  }
  return 0;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  // Idle jiffies = idle + iowait time
  std::ifstream stream(kProcDirectory + kStatFilename);
  string line;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    string cpu;
    long value;
    long idle_jiffies = 0;
    
    // Skip first field (cpu)
    linestream >> cpu;
    
    // Skip user, nice, system
    for (int i = 0; i < 3; i++) {
      linestream >> value;
    }
    
    linestream >> value;  // idle
    idle_jiffies += value;
    linestream >> value;  // iowait
    idle_jiffies += value;
    
    return idle_jiffies;
  }
  return 0;
}

// TODO: Read and return CPU utilization
vector<float> LinuxParser::CpuUtilization() { 
  vector<float> cpu_utilization;
  std::ifstream stream(kProcDirectory + kStatFilename);
  string line;
  
  if (stream.is_open()) {
    // cpu (aggregate)
    // cpu0 ...
    // cpu1 ...
    // ...
    // Parse only the first cpu (aggregate)
    if (std::getline(stream, line)) {
      if (line.substr(0, 3) == "cpu") {
        std::istringstream linestream(line);
        string cpu;
        vector<long> values;
        long value;
        
        linestream >> cpu; // CPU id
        
        while (linestream >> value) {
          values.push_back(value);
        }
        
        // Calculate utilization for CPU_i
        if (values.size() >= 7) {  
          long total_time = 0;
          for (int i = 0; i < 7; i++) {
            total_time += values[i];
          }
          long idle_time = values[3] + values[4]; 
          cpu_utilization.push_back(1.0 - (static_cast<float>(idle_time) / total_time));
        }
      }
    }
  }
  return cpu_utilization;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  std::ifstream stream(kProcDirectory + kStatFilename);
  string line;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      // Note that this line returns the total number of forks since system startup
      // not the current number or running processes
        
      if (line.substr(0, 9) == "processes") {
        std::istringstream linestream(line);
        string key;
        int value;
        linestream >> key >> value;
        return value;
      }
    }
  }
  return 0;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  return Pids().size();
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  string line;
  if (stream.is_open()) {
    std::getline(stream, line);
    return line;
  }
  return string();
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  string line;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      if (line.substr(0, 6) == "VmSize") {
        std::istringstream linestream(line);
        string key, value, unit;
        linestream >> key >> value >> unit;
        return std::to_string(std::stol(value) / 1024);
      }
    }
  }
  return string();
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  string line;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      if (line.substr(0, 4) == "Uid:") {
        std::istringstream linestream(line);
        string key, value;
        linestream >> key >> value;
        return value;
      }
    }
  }
  return string();
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  std::ifstream stream(kPasswordPath);
  string line;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      string name, x, id;
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> name >> x >> id;
      if (id == uid) {
        return name;
      }
    }
  }
  return string();
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  string line;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);

    string value;
    for (int i = 0; i < 21; i++) {
      linestream >> value;
    }
    // Read starttime (22nd value)
    linestream >> value;
    long starttime = std::stol(value);
    // Convert to seconds and subtract from system uptime
    return UpTime() - (starttime / sysconf(_SC_CLK_TCK));
  }
  return 0;
}
