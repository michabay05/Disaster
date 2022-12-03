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

long long Time::now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

uint32_t randomState = 1804289383;

uint32_t random32() {
    uint32_t number = randomState;

    // XOR shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    // Update random number state
    randomState = number;

    // Return random number
    return number;
}

uint64_t random64() {
    uint64_t rand1, rand2, rand3, rand4;
    rand1 = (uint64_t)(random32() & 0xFFFF);
    rand2 = (uint64_t)(random32() & 0xFFFF);
    rand3 = (uint64_t)(random32() & 0xFFFF);
    rand4 = (uint64_t)(random32() & 0xFFFF);
    return rand1 | (rand2 << 16) | (rand3 << 32) | (rand4 << 48);
}

uint64_t genRandomMagic() { return random64() & random64() & random64(); }
