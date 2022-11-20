#pragma once

#include <chrono>

struct Time {
  static long long startTime, endTime;

public:
  static void start();
  static long long end();
};
