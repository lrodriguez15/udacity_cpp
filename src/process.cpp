#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// Add constructor for Process
Process::Process(int pid) : pid_(pid) {}

//  Return this process's ID
int Process::Pid() const { return pid_; }

//  Return this process's CPU utilization
float Process::CpuUtilization() const { return cpu_; }

void Process::CpuUtilization(long active_jiffles, long system_jiffles) {
  cpu_ = (static_cast<float>(active_jiffles)/ LinuxParser::ClockTicksPerSecond())/UpTime();
  cached_active_jiffles_ = active_jiffles;
  cached_system_jiffles_ = system_jiffles;
}

//  Return the command that generated this process
string Process::Command() const { return LinuxParser::Command(Pid()); }

//  Return this process's memory utilization
string Process::Ram() const { return LinuxParser::Ram(Pid()); }

//  Return the user (name) that generated this process
string Process::User() const { return LinuxParser::User(Pid()); }

//  Return the age of this process (in seconds)
long int Process::UpTime() const { return LinuxParser::UpTime(Pid()); }

//  Overload the "greater than" comparison operator for Process objects
bool Process::operator>(const Process& a) const {
  return CpuUtilization() > a.CpuUtilization();
}

//  Overload the "less than" comparison operator for Process objects
bool Process::operator<(const Process& a) const {
  return CpuUtilization() < a.CpuUtilization();
}