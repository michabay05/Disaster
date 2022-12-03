#pragma once

#include "defs.hpp"
#include <chrono>

struct Time {
    static long long startTime, endTime;

    static void start();
    static long long end();
    static long long now();
};


uint32_t random32();
uint64_t random64();
uint64_t genRandomMagic();
