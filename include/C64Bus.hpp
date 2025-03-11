#pragma once

#include <cstdint>
#include <vector>
#include <fstream>
#include <cia1.hpp>
#include <cia2.hpp>
#include <vic.hpp>
#include <input.hpp>
#include <sid.hpp>
#include <bus.hpp>

class VIC;
class CIA1;
class CIA2;
class Input;

class C64Bus : public Bus {
public:
    C64Bus();
    ~C64Bus();

    void write(uint16_t addr, uint8_t data) override;
    uint8_t read(uint16_t addr) override;

    uint8_t readCharRom(uint16_t addr);

    uint8_t handleIoRead(uint16_t addr);
    void handleIoWrite(uint16_t addr, uint8_t data);

    void loadC64rom(const char *filename);
    void loadCharacterRom(const char *filename);
    void loadCartridge(const char *filename);

    uint8_t dataDirectionRegister;
    uint8_t dataRegister = 0b00000111;

    CIA1 *cia1;
    CIA2 *cia2;
    VIC *vic;
    SID *sid;
    Input *input;
// private:
    std::ofstream ramFile;
    uint8_t ram[0xFFFF];
    uint8_t basicRom[0x2000];
    uint8_t kernalRom[0x2000];
    uint8_t charRom[0x1000];
    uint8_t colorRam[0x0400];
    uint8_t cartridge[0x8000];
    bool cartridgeLoaded = false;
};
