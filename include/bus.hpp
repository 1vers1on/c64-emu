#pragma once

#include <cstdint>
#include <vector>
#include <fstream>
#include <cia1.hpp>
#include <cia2.hpp>
#include <vic.hpp>

class VIC;

class Bus {
public:
    Bus();
    ~Bus();

    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);

    uint8_t readCharRom(uint16_t addr);

    void writeWord(uint16_t addr, uint16_t data);
    uint16_t readWord(uint16_t addr);

    void writeBytes(uint16_t addr, const uint8_t *data, uint16_t size);

    uint8_t handleIoRead(uint16_t addr);
    void handleIoWrite(uint16_t addr, uint8_t data);

    void loadC64rom(const char *filename);
    void loadCharacterRom(const char *filename);

    uint8_t dataDirectionRegister;
    uint8_t dataRegister = 0b00000111;

    CIA1 *cia1;
    CIA2 *cia2;
    VIC *vic;
private:
    std::ofstream ramFile;
    uint8_t ram[0xFFFF];
    uint8_t basicRom[0x2000];
    uint8_t kernelRom[0x2000];
    uint8_t charRom[0x1000];
    uint8_t colorRam[0x0400];
};
