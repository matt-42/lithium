#pragma once

#include <chrono>

namespace li
{

class timer {
public:
  inline void start() { start_ = std::chrono::high_resolution_clock::now(); }
  inline void end() { end_ = std::chrono::high_resolution_clock::now(); }

  inline unsigned long us() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_).count();
  }

  inline unsigned long ms() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end_ - start_).count();
  }

  inline unsigned long ns() const {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_).count();
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_, end_;
};

}
