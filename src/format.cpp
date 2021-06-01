#include "format.h"

#include <string>

using std::string;

// Helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  const int hours = seconds / 60 / 60;
  string str_hours =
      (hours < 10) ? "0" + std::to_string(hours) : std::to_string(hours);
  const int secs = seconds % 60;
  string str_secs =
      (secs < 10) ? "0" + std::to_string(secs) : std::to_string(secs);
  const int mins = (seconds - hours * 3600 - secs) / 60;
  string str_mins =
      (mins < 10) ? "0" + std::to_string(mins) : std::to_string(mins);
  return (str_hours + ":" + str_mins + ":" +
          str_secs);
}