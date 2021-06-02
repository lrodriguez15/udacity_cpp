#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

using std::ifstream;
using std::istringstream;
using std::stof;
using std::stoi;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

std::vector<std::string> StringToVector(string filename) {
  std::vector<string> result;
  std::string line, value;
  std::ifstream f_stream(filename);

  if (f_stream) {
    getline(f_stream, line);
    std::istringstream lne_stream(line);
    while (lne_stream >> value) {
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
  string val;
  std::ifstream f_stream(kOSPath);
  if (f_stream.is_open()) {
    bool exitouterloop = false; 
    while (std::getline(f_stream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream lne_stream(line);
      while (lne_stream >> key >> val) {
        if (key == "PRETTY_NAME") {
          std::replace(val.begin(), val.end(), '_', ' ');
          exitouterloop = true; 
          break;
        }
      }
      if(exitouterloop)
      {
        break;
      }
    }
  }
  f_stream.close();
  return val;
}

// An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream f_stream(kProcDirectory + kVersionFilename);
  if (f_stream.is_open()) {
    std::getline(f_stream, line);
    std::istringstream lne_stream(line);
    lne_stream >> os >> version >> kernel;
  }
  f_stream.close();
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
  auto total_memory =
      static_cast<float>(LinuxParser::ReadProcessMemory("MemTotal"));
  auto avail_memory =
      static_cast<float>(LinuxParser::ReadProcessMemory("MemFree"));
  auto result = (total_memory - avail_memory) / total_memory;
  return result;
}

// Read and return the system memory utilization
long LinuxParser::UpTime() {
  string line;
  string str_value;
  long value = 0;
  std::ifstream f_stream(kProcDirectory + kUptimeFilename);

  if (f_stream.is_open()) {
    getline(f_stream, line);
    std::istringstream lne_stream(line);
    lne_stream >> str_value;
    value = stol(str_value);
  }
  f_stream.close();
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
  long result = user + nice + system + idle + iowait + irq + softirq + steal;
  return result;
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

    std::istringstream lne_stream(line);
    lne_stream >> val;
    for (int i = CPUStates::kUser_; i < CPUStates::kGuestNice_; i++) {
      lne_stream >> val;
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
  string val;
  std::ifstream f_stream(kProcDirectory + kStatFilename);
  if (f_stream) {
    bool exitouterloop = false;
    while (getline(f_stream, line)) {
      std::istringstream lne_stream(line);
      while (lne_stream >> key >> val) {
        if (key == "processes") {
          exitouterloop = true;
          break;
        }
      }
      if (exitouterloop) {
        break;
      }
    }
  }
  f_stream.close();
  return stoi(val);
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  int val;
  ifstream f_stream(kProcDirectory + kStatFilename);
  if (f_stream) {
    bool exitouterloop = false;
    while (getline(f_stream, line)) {
      std::istringstream lne_stream(line);
      while (lne_stream >> key >> val) {
        if (key == "procs_running") {
          exitouterloop = true;
          break;
        }
      }
      if (exitouterloop) {
        break;
      }
    }
    f_stream.close();
  }
  return val;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::string line;
  std::string str_pid = to_string(pid);
  std::ifstream f_stream(kProcDirectory + str_pid + kCmdlineFilename);

  if (f_stream) {
    getline(f_stream, line);
  }
  f_stream.close();
  return line;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  const auto kb2mb = 1024;
  const auto mbMem = (ReadProcessID(pid, "VmRSS") / kb2mb);
  return to_string(mbMem);
}

// TODO: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  const auto uid = ReadProcessID(pid, "Uid");
  return to_string(uid);
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  std::string key = LinuxParser::Uid(pid);
  std::string line, uid_str, user;

  std::ifstream f_stream(kPasswordPath);

  if (f_stream) {
    bool exitouterloop = false;
    while (getline(f_stream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), ':', ' ');
      std::replace(line.begin(), line.end(), 'x', ' ');
      std::istringstream lne_stream(line);

      while (lne_stream >> user >> uid_str) {
        if (uid_str == key) {
          exitouterloop = true;
          break;
        };
      }
      if (exitouterloop) {
        break;
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
  auto string_pid = to_string(pid);
  std::vector<string> result =
      StringToVector(kProcDirectory + string_pid + kStatFilename);
  if (result.size() >= 21) {
    const auto start = stol(result[21]);
    return UpTime() - (start / ClockTicksPerSecond());
  }

  return 0;
}

// Helper function to determine process information (parsing)
long ReadProcessInfo(const std::string filename, const std::string search_key) {
  std::string line, key, str_value;
  std::ifstream f_stream(filename);
  long result = 0;

  if (f_stream) {
    bool exitouterloop = false;
    while (getline(f_stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream lne_stream(line);

      while (lne_stream >> key >> str_value) {
        if (key == search_key) {
          exitouterloop = true;
          break;
        }
      }
      if (exitouterloop) {
        result = stol(str_value);
        break;
      } else
        result = 0;
    }
  }
  f_stream.close();
  return result;
}

// Helper function to read process memory
long LinuxParser::ReadProcessMemory(const std::string search_key) {
  std::string filename = kProcDirectory + kMeminfoFilename;
  auto result = ReadProcessInfo(filename, search_key);
  return result;
}

// Helper function to read process ID
long LinuxParser::ReadProcessID(const int &pid, const std::string search_key) {
  std::string filename = kProcDirectory + std::to_string(pid) + kStatusFilename;
  auto result = ReadProcessInfo(filename, search_key);
  return result;
}