#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(int pid);
  int Pid() const;
  std::string User() const;
  std::string Command() const;
  float CpuUtilization() const;
  std::string Ram() const;
  long int UpTime() const;
  bool operator<(Process const& a) const;
  bool operator>(Process const& a) const;
  void CpuUtilization(long active_ticks, long system_ticks);

  // Declare private members
 private:
  int pid_;
  float cpu_;
  long cached_active_jiffles_{0};
  long cached_idle_jiffles_{0};
  long cached_system_jiffles_{0};
};

#endif