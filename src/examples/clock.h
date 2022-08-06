#pragma once

#include <chrono>

namespace example {

struct Clock {
  std::chrono::nanoseconds start_time;
  std::chrono::nanoseconds current_time;
  std::chrono::nanoseconds delta_time;

  Clock();

  void Tick();
  float GetTime() const;
  float GetDeltaTime() const;

  static std::chrono::nanoseconds GetCurrentNanoseconds();
};

}  // namespace example
