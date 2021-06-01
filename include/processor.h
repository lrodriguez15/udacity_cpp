#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

  // DONE: Declare any necessary private members
 private:
  long cached_active_ticks_{0};
  long cached_idle_ticks_{0};
};

#endif