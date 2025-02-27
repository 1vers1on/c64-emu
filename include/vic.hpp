#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>
#include <bus.hpp>
#include <cpu.hpp>

#define PAL 1

class Bus;

class VIC {
public:
    VIC(Bus* bus);

    ~VIC();

    uint8_t registers[0x2F] = {}; // 64 VIC-II registers

    uint8_t read(uint16_t addr);

    void write(uint16_t addr, uint8_t value);

    void checkInterrupts();

    void tick();

    uint32_t getColor(uint8_t color);
    void setFramebufferCallback(std::function<void(std::array<uint32_t, 40 * 25 * 8 * 8>&)> callback);

    void setCpu(CPU* cpu);

    bool needsRender = false;

    std::array<uint32_t, 40 * 25 * 8 * 8> screen = {};

    uint16_t bankAddress = 0x0000;


private:
    void handleRasterInterrupts();
    void handleDMASteal();
    void renderScanline();
    CPU* cpu;
    Bus* bus;
    std::function<void(std::array<uint32_t, 40 * 25 * 8 * 8>&)> framebufferCallback;
    uint16_t rasterLine = 0; // current raster line
    size_t rasterCycle = 0; // current raster cycle
    size_t cycleCounter = 0; // cycle counter

    bool bitmapMode = false;
    bool multiColorMode = false;

    uint16_t charMemOffset = 0;
    uint16_t screenMemoryOffset = 0;
    uint16_t bitmapOffset = 0;
};
    