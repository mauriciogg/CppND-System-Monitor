#ifndef SYSTEM_MONITOR_PROCESS_H
#define SYSTEM_MONITOR_PROCESS_H

#include <string>
#include <sys/time.h>

#include "linux_parser.h"
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process() = default;
  Process(int pid);
  Process(const Process& other);
  Process(Process&& other);
  ~Process() = default;

  Process& operator=(const Process& other);
  Process& operator=(Process&& other);

  int Pid() const;
  std::string User();
  std::string Command();
  float CpuUtilization();
  std::string Ram();
  long int UpTime();
  struct timespec StartTime();
  bool operator<(Process& a);

  // TODO: Declare any necessary private members
 private:
    // These fields don't change so it makes sense to cache them during initialization
    int pid_;
    std::string user_;
    std::string command_;
    
    // CPU utilization tracking
    long prev_jiffies_{0};
    struct timespec prev_time_{};
    struct timespec start_time_{};
    float cpu_utilization_{0.0};  // Cached CPU utilization value (for sorting with stable values)
};

#endif
