#include "misc.h"

long long Time::startTime = 0LL;
long long Time::endTime = 0LL;
void Time::start() {
  startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::high_resolution_clock::now().time_since_epoch())
              .count();
}

long long Time::end() {
  endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch())
            .count();
  return endTime - startTime;
}
