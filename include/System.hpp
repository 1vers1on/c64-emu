#pragma once

#include <bus.hpp>
#include <cpu.hpp>
#include <cia1.hpp>
#include <cia2.hpp>
#include <vic.hpp>
#include <sid.hpp>
#include <input.hpp>

class System {
public:
    System();
    ~System();

    void loadRoms(const std::string& kernalAndBasicRom, const std::string& characterRom);

    void powerOn();
    void reset();

    void step();

    CPU* cpu;
    Bus* bus;
    CIA1* cia1;
    CIA2* cia2;
    VIC* vic;
    SID* sid;
    Input* input;
};
