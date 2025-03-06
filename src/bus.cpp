#include "chrom.h"
#include "kernal.h"

#include <arpa/inet.h> // For ntohl
#include <bus.hpp>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

Bus::Bus() {
    for(int i = 0; i < c64_chrom_bin_len; i++) {
        charRom[i] = c64_chrom_bin[i];
    }

    for(int i = 0; i < c64_kernal_bin_len; i++) {
        basicRom[i] = c64_kernal_bin[i];
    }

    for(int i = 0; i < 0xFFFF; i++) {
        ram[i] = 0x00;
    }
    dataDirectionRegister = 0b11111000;
    dataRegister = 0b00000111;
}

Bus::~Bus() {
}

struct CRTHeader {
    char signature[16];
    uint32_t headerLength;
    uint16_t version;
    uint16_t cartridgeType;
    uint8_t exromLine;
    uint8_t gameLine;
    uint8_t reserved[7];
    char name[32];
};

struct CHIPPacket {
    char signature[4];
    uint32_t length;
    uint16_t type;
    uint16_t bank;
    uint16_t loadAddress;
    uint16_t dataLength;
    uint8_t* data;
};

void Bus::loadCartridge(const char* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    if(!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    size_t size = 0;
    file.seekg(0, std::ios::end);
    size = file.tellg();
    file.seekg(0, std::ios::beg);

    CRTHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(CRTHeader));

    header.headerLength = ntohl(header.headerLength);

    std::cout << "Raw header bytes: ";
    for(size_t i = 0; i < sizeof(CRTHeader); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (int)((unsigned char*)&header)[i] << " ";
    }
    std::cout << std::dec << std::endl;

    if(std::string(header.signature, 16) != "C64 CARTRIDGE   ") {
        std::cerr << "Invalid CRT file" << std::endl;
        return;
    }

    file.seekg(header.headerLength - sizeof(CRTHeader), std::ios::cur);

    std::vector<CHIPPacket> packets;
    while(file.tellg() < size) {
        CHIPPacket packet;
        file.read(reinterpret_cast<char*>(&packet), sizeof(CHIPPacket));
        packet.length = ntohl(packet.length);
        packet.bank = ntohs(packet.bank);
        packet.loadAddress = ntohs(packet.loadAddress);
        packet.dataLength = ntohs(packet.dataLength);
        packet.data = new uint8_t[packet.dataLength];
        file.read(reinterpret_cast<char*>(packet.data), packet.dataLength);
        packets.push_back(packet);
    }

    for(CHIPPacket packet : packets) {
        if(std::string(packet.signature, 4) == "CHIP") {
            std::cout << "CHIP packet" << std::endl;
            // print the data
            for(size_t i = 0; i < packet.dataLength; ++i) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)packet.data[i]
                          << " ";
            }
            std::cout << std::dec << std::endl;
            // write the header manually
            // write CBM80

            // writeBytes(packet.loadAddress, packet.data, packet.dataLength);
        }
    }

    cartridgeLoaded = true;
}

void Bus::write(uint16_t addr, uint8_t data) {
#ifndef NO_MMIO
    if(addr == 0x0000) dataDirectionRegister = data;
    if(addr == 0x0001) dataRegister = data;

    if(addr >= 0x8000 && addr <= 0x9FFF) {
        if(cartridgeLoaded) {
            // std::cerr << "Attempted to write to cartridge ROM" << std::endl;
            return;
        }
    }

    if((dataRegister & 0b011) == 0b00) {
        ram[addr] = data;
        return;
    }

    if(addr >= 0xA000 && addr <= 0xBFFF) {
        if((dataRegister & 0b011) == 0b01 || (dataRegister & 0b011) == 0b10) {
            ram[addr] = data;
        } else {
            // std::cerr << "Attempted to write to ROM" << std::endl;
        }
    }
    if(addr >= 0xE000 && addr <= 0xFFFF) {
        if((dataRegister & 0b011) == 0b01) {
            ram[addr] = data;
        } else {
            // std::cerr << "Attempted to write to ROM" << std::endl;
        }
    }
    if(addr >= 0xD000 && addr <= 0xDFFF) {
        if((dataRegister & 0b100) == 0b100) {
            handleIoWrite(addr, data);
        } else {
            // std::cerr << "Attempted to write to ROM" << std::endl;
        }
    }
#endif
    ram[addr] = data;
}

uint8_t Bus::read(uint16_t addr) {
#ifndef NO_MMIO
    if(addr == 0x0000) return dataDirectionRegister;
    if(addr == 0x0001) return dataRegister;

    if(addr >= 0x8000 && addr <= 0x9FFF) {
        if(cartridgeLoaded) {
            return cartridge[addr - 0x8000];
        }
    }

    if((dataRegister & 0b011) == 0b00) {
        return ram[addr];
    }

    if(addr >= 0xA000 && addr <= 0xBFFF) {
        if((dataRegister & 0b011) == 0b01 || (dataRegister & 0b011) == 0b10) {
            return ram[addr];
        } else {
            return basicRom[addr - 0xA000];
        }
    }
    if(addr >= 0xE000 && addr <= 0xFFFF) {
        if((dataRegister & 0b011) == 0b01) {
            return ram[addr];
        } else {
            return kernalRom[addr - 0xE000];
        }
    }
    if(addr >= 0xD000 && addr <= 0xDFFF) {
        if((dataRegister & 0b100) == 0b100) {
            return handleIoRead(addr);
        } else {
            return charRom[addr - 0xD000];
        }
    }
#endif
    return ram[addr];
}

void Bus::writeWord(uint16_t addr, uint16_t data) {
    write(addr, data & 0xFF);
    write(addr + 1, data >> 8);
}

uint16_t Bus::readWord(uint16_t addr) {
    return read(addr) | (read(addr + 1) << 8);
}

void Bus::writeBytes(uint16_t addr, const uint8_t* data, uint16_t size) {
    for(int i = 0; i < size; i++) {
        ram[addr + i] = data[i];
    }
}

uint8_t Bus::handleIoRead(uint16_t addr) {
    if(addr >= 0xD400 && addr < 0xD800) return sid->read(addr);
    if(addr >= 0xD800 && addr < 0xDBFF) return colorRam[addr - 0xD800];
    if(addr >= 0xD000 && addr < 0xD400) return vic->read(addr);
    if(addr >= 0xDC00 && addr < 0xDD00) return cia1->read(addr);
    if(addr >= 0xDD00 && addr < 0xDE00) return cia2->read(addr);
    return 0;
}

void Bus::handleIoWrite(uint16_t addr, uint8_t data) {
    if(addr >= 0xD400 && addr < 0xD800) sid->write(addr, data);
    if(addr >= 0xD800 && addr < 0xDBFF) {
        colorRam[addr - 0xD800] = data;
    }
    if(addr >= 0xD000 && addr < 0xD400) vic->write(addr, data);
    if(addr >= 0xDC00 && addr < 0xDD00) cia1->write(addr, data);
    if(addr >= 0xDD00 && addr < 0xDE00) cia2->write(addr, data);
}

void Bus::loadC64rom(const char* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    if(!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    file.read(reinterpret_cast<char*>(basicRom), 0x2000);
    file.read(reinterpret_cast<char*>(kernalRom), 0x2000);

    file.close();
}

void Bus::loadCharacterRom(const char* filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    if(!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    file.read(reinterpret_cast<char*>(charRom), 0x1000);

    file.close();
}

uint8_t Bus::readCharRom(uint16_t addr) {
    return charRom[addr];
}
