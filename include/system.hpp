#pragma once

#include <bus.hpp>
#include <cpu.hpp>
#include <cia1.hpp>
#include <cia2.hpp>
#include <vic.hpp>
#include <sid.hpp>
#include <input.hpp>
#include <serial_bus.hpp>
#include <chrono>

class System {
public:
    System();
    ~System();

    void loadRoms(const std::string& kernalAndBasicRom, const std::string& characterRom);

    void powerOn();
    void reset();

    void step();

    int clockSpeed;

    CPU* cpu;
    Bus* bus;
    CIA1* cia1;
    CIA2* cia2;
    VIC* vic;
    SID* sid;
    Input* input;
    SerialBus* serialBus;

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    std::chrono::duration<double> accumulatedTime;
    std::chrono::duration<double> timeThreshold;
    size_t cycles;
};
