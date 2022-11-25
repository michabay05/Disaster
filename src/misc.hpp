#pragma once

#include <chrono>
#include "defs.hpp"

struct Time {
  static long long startTime, endTime;

  static void start();
  static long long end();
};

inline bool addWithConstraint(int& num, const int addend, const int limit) {
	if (num + addend < limit) {
		num += addend;
		return true;
	} else 
		return false;
}

U64 random64();
