#include "system.h"

#include <unistd.h>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

//  Return the system's CPU
Processor& System::Cpu() { return cpu_; }

//  Return a container composed of the system's processes
vector<Process>& System::Processes() {
  vector<int> pids{LinuxParser::Pids()};

  // Create a set
  set<int> existing_pids;
  for (Process const& process : processes_) {
    existing_pids.insert(process.Pid());
  }
  for (int pid : pids) {
    if (existing_pids.find(pid) == existing_pids.end())
      processes_.emplace_back(pid);
  }
  
  // Update the CPU utilization for the identified processes
  for (auto& process : processes_) {
    process.CpuUtilization(LinuxParser::ActiveJiffies(process.Pid()),
                           LinuxParser::Jiffies());
  }
  // We needed to define the > operator for this to work
  std::sort(processes_.begin(), processes_.end(), std::greater<Process>());
  return processes_;
}

//  Return the system's kernel identifier (string)
std::string System::Kernel() { return LinuxParser::Kernel(); }

//  Return the system's memory utilization
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

//  Return the operating system name
std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

//  Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

//  Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

//  Return the number of seconds since the system started running
long int System::UpTime() { return LinuxParser::UpTime(); }