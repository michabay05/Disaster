#include "misc.hpp"

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

U64 random64() {
  U64 rand1, rand2, rand3, rand4;
  rand1 = (U64)(rand() & 0xFFFF);
  rand2 = (U64)(rand() & 0xFFFF);
  rand3 = (U64)(rand() & 0xFFFF);
  rand4 = (U64)(rand() & 0xFFFF);
  return rand1 | (rand2 << 16) | (rand3 << 32) | (rand4 << 48);
}
