#pragma once

#include <cstdint>
#include <cstddef>

class VIC {
public:
    VIC();

    ~VIC();

    uint8_t registers[0x2F] = {}; // 64 VIC-II registers

    uint8_t read(uint16_t addr);

    void write(uint16_t addr, uint8_t value);

    void checkInterrupts();

    void tick(size_t cycles);

    bool irqPending = false; // interrupt pending flag
    void setIrq(uint8_t bit);

private:
    uint16_t rasterLine = 0; // current raster line
    size_t cycleCounter = 0; // cycle counter
};
    