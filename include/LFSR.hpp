#pragma once

#include <cstdint>

class LFSR {
public:
    LFSR(uint32_t seed = 0xACE1) : state(seed) {}

    uint8_t nextBit();
    void reset(uint32_t seed);

private:
    uint32_t state;
};
