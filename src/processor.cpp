#include "processor.h"

#include "linux_parser.h"

// TReturn the aggregate CPU utilization
float Processor::Utilization() {
  float utilization{0};
  long active_jiffles = LinuxParser::ActiveJiffies();
  long idle_jiffles = LinuxParser::IdleJiffies();
  long duration_active{active_jiffles - cached_active_jiffles_};
  long duration_idle{idle_jiffles - cached_idle_jiffles_};
  long duration{duration_active + duration_idle};
  utilization = static_cast<float>(duration_active) / duration;
  cached_active_jiffles_ = active_jiffles;
  cached_idle_jiffles_ = idle_jiffles;
  return utilization;
}