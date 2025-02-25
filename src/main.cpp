#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cpu.hpp>
#include <chrono>
#include <thread>

// structure to represent the iNES header (16 bytes)
struct NesHeader{
    uint8_t magic[4];       // 0x4E, 0x45, 0x53, 0x1A
    uint8_t prg_size;       // PRG ROM in 16K
    uint8_t chr_size;       // CHR ROM in 8K, 0 -> using CHR RAM
    uint8_t flag6;
    uint8_t flag7;
    uint8_t prg_ram_size;   // PRG RAM in 8K
    uint8_t flag9;
    uint8_t flag10;         // unofficial
    uint8_t reserved[5];    // reserved
};

int main() {
    std::ifstream file("nestest.nes", std::ios::binary | std::ios::in);
    NesHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(NesHeader));
    if (header.flag6 & 0x04) {
        file.seekg(512, std::ios::cur);
    }

    if (header.flag7 == 0x44) {
        header.flag7 = 0;
    }

    int mapper = ((header.flag6 & 0xf0) >> 4) + ((header.flag7 & 0xf0));

    int prgSize = header.prg_size * 0x4000;
    int chrSize = header.chr_size * 0x2000;

    uint8_t* prg_rom = new uint8_t[prgSize];
    uint8_t* chr_rom = new uint8_t[chrSize];

    file.read(reinterpret_cast<char*>(prg_rom), prgSize);
    file.read(reinterpret_cast<char*>(chr_rom), chrSize);

    CPU cpu;

    if (mapper == 0) {
        cpu.bus->writeBytes(0x8000, prg_rom, prgSize);
        if (prgSize == 0x4000) {
            cpu.bus->writeBytes(0xC000, prg_rom, prgSize);
        }
    } else {
        std::cerr << "Unsupported mapper: " << mapper << std::endl;
    }

    cpu.powerOn();

    std::cout << std::hex << static_cast<int>(cpu.bus->readWord(0xFFFC)) << std::endl;

    std::cout << "rom loaded and mapped successfully!" << std::endl;

    while (true) {
        cpu.executeOnce();
    }

    return 0;
}
