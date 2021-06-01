#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

using std::ifstream;
using std::istringstream;
using std::stof;
using std::string;
using std::to_string;
using std::vector;

std::vector<std::string> StringToVector(string filename) {
  std::vector<string> result;
  std::string line, value;
  std::ifstream f_stream(filename);

  if (f_stream) {
    getline(f_stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      result.push_back(value);
    }
  }
  f_stream.close();

  return result;
}

//  An example of how to read data from the filesystem
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

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  stream.close();
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR *directory = opendir(kProcDirectory.c_str());
  struct dirent *file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  auto total_memory = static_cast<float>(LinuxParser::ReadProcMem("MemTotal"));
  auto avail_memory =
      static_cast<float>(LinuxParser::ReadProcMem("MemAvailable"));
  auto result = (total_memory - avail_memory) / total_memory;
  return result;
}

// Read and return the system memory utilization
long LinuxParser::UpTime() {
  string line;
  string str_value;
  long value = 0;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);

  if (filestream.is_open()) {
    getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> str_value;
    value = stol(str_value);
  }
  filestream.close();
  return value;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  auto total_jiffies = ActiveJiffies() - IdleJiffies();
  return total_jiffies;
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  std::string filename = kProcDirectory + to_string(pid) + kStatFilename;
  std::vector<string> vstr = StringToVector(filename);
  long result = 0;
  if (vstr.size() >= 21) {
    result =
        (stol(vstr[13]) + stol(vstr[14]) + stol(vstr[15]) + stol(vstr[16]));
  }
  return result;
}
// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  const auto cpu_stats = LinuxParser::CpuUtilization();

  const auto user = stol(cpu_stats[LinuxParser::kUser_]);
  const auto nice = stol(cpu_stats[LinuxParser::kNice_]);
  const auto system = stol(cpu_stats[LinuxParser::kSystem_]);
  const auto idle = stol(cpu_stats[LinuxParser::kIdle_]);

  const auto iowait = stol(cpu_stats[LinuxParser::kIOwait_]);
  const auto irq = stol(cpu_stats[LinuxParser::kIRQ_]);
  const auto softirq = stol(cpu_stats[LinuxParser::kSoftIRQ_]);
  const auto steal = stol(cpu_stats[LinuxParser::kSteal_]);
  return (user + nice + system + idle + iowait + irq + softirq + steal);
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  const auto cpu_stats = LinuxParser::CpuUtilization();
  const auto idle = stol(cpu_stats[LinuxParser::kIdle_]);
  const auto iowait = stol(cpu_stats[LinuxParser::kIOwait_]);
  return idle + iowait;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  std::vector<string> result;
  std::string line, val;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream) {
    getline(stream, line);

    std::istringstream linestream(line);
    linestream >> val;
    for (int ii = 0; ii < CPUStates::kGuestNice_; ii++) {
      linestream >> val;
      result.push_back(val);
    }
  }
  stream.close();
  return result;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::string line;
  std::string key;
  int value;
  std::ifstream f_stream(kProcDirectory + kStatFilename);
  if (f_stream) {
    while (getline(f_stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") return value;
      }
    }
  }
  f_stream.close();
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  int value;
  ifstream f_stream(kProcDirectory + kStatFilename);
  if (f_stream) {
    while (getline(f_stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") return value;
      }
    }
  }
  f_stream.close();
  return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::string line;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kCmdlineFilename);

  if (filestream) {
    getline(filestream, line);
  }
  filestream.close();
  return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  const auto kb2mb = 1024;
  const auto mbMem = (ReadProcPID(pid, "VmRSS") / kb2mb);
  return to_string(mbMem);
}

// TODO: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  const auto uid = ReadProcPID(pid, "Uid");
  return to_string(uid);
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  std::string key = LinuxParser::Uid(pid);
  std::string line, uid_str, user;

  std::ifstream f_stream(kPasswordPath);

  if (f_stream) {
    while (getline(f_stream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), 'x', ' ');
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);

      while (linestream >> user >> uid_str) {
        if (uid_str == key) return user;
      }
    }
  }

  f_stream.close();
  return user;
}

// Determine the ticks per second as a vector of string
int LinuxParser::ClockTicksPerSecond() { return sysconf(_SC_CLK_TCK); }

//  Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::vector<string> result =
      StringToVector(kProcDirectory + to_string(pid) + kStatFilename);

  if (result.size() >= 21) {
    const auto starttime = stol(result[21]);
    return UpTime() - (starttime / ClockTicksPerSecond());
  }

  return 0;
}

long ReadProcInfo(const std::string filename, const std::string &search_key) {
  std::string line, key, str_value;
  std::ifstream f_stream(filename);

  if (f_stream) {
    while (getline(f_stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);

      while (linestream >> key >> str_value) {
        if (key == search_key) {
          return stol(str_value);
        }
      }
    }
  }
  f_stream.close();
  return 0;
}

long LinuxParser::ReadProcMem(const std::string &search_key) {
  std::string filename = kProcDirectory + kMeminfoFilename;
  return ReadProcInfo(filename, search_key);
}

long LinuxParser::ReadProcPID(const int &pid, const std::string &search_key) {
  std::string filename = kProcDirectory + std::to_string(pid) + kStatusFilename;
  return ReadProcInfo(filename, search_key);
}