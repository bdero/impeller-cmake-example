// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "clock.h"

namespace example {

Clock::Clock()
    : start_time(GetCurrentNanoseconds()),
      current_time(GetCurrentNanoseconds()) {}

void Clock::Tick() {
  auto time = GetCurrentNanoseconds();
  delta_time = time - current_time;
  current_time = time;
}

float Clock::GetTime() const {
  return static_cast<float>((current_time - start_time).count()) / 1000000000;
}

float Clock::GetDeltaTime() const {
  return static_cast<float>(delta_time.count()) / 1000000000;
}

std::chrono::nanoseconds Clock::GetCurrentNanoseconds() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::system_clock::now().time_since_epoch());
}

}  // namespace example
