#include <LFSR.hpp>

uint8_t LFSR::nextBit() {
    uint8_t newBit = ((state >> 22) ^ (state >> 16)) & 1;
    state = (state << 1) | newBit;
    return state & 1;
}

void LFSR::reset(uint32_t seed) {
    state = seed;
}
