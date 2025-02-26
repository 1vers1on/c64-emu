#include <bus.hpp>
#include <cstdint>
#include <iostream>
#include <fstream>

Bus::Bus() {
    cia1 = new CIA1();
    vic = new VIC();
    ramFile.open("ram.bin", std::ios::binary | std::ios::out);

    for (int i = 0; i < 0xFFFF; i++) {
        ram[i] = 0x00;
        ramFile.write(reinterpret_cast<char*>(&ram[i]), 1);
    }
    dataDirectionRegister = 0b11111000;
    dataRegister = 0b00000111;
}

Bus::~Bus() {
}

void Bus::write(uint16_t addr, uint8_t data) {
    // std::cout << "Writing " << std::hex << static_cast<int>(data) << " to address: " << addr << std::dec << "\n";
    if (addr == 0x0000) dataDirectionRegister = data;
    if (addr == 0x0001) dataRegister = data;

    if ((dataRegister & 0b011) == 0b00) {
        ram[addr] = data;
        ramFile.seekp(addr);
        ramFile.write(reinterpret_cast<char*>(&data), 1);
        return;
    }

    if (addr >= 0xA000 && addr <= 0xBFFF) {
        if ((dataRegister & 0b011) == 0b01 || (dataRegister & 0b011) == 0b10) {
            ram[addr] = data;
            ramFile.seekp(addr);
            ramFile.write(reinterpret_cast<char*>(&data), 1);
        } else {
            std::cerr << "Attempted to write to ROM" << std::endl;
        } 
    }
    if (addr >= 0xE000 && addr <= 0xFFFF) {
        if ((dataRegister & 0b011) == 0b01) {
            ram[addr] = data;
            ramFile.seekp(addr);
            ramFile.write(reinterpret_cast<char*>(&data), 1);
        } else {
            std::cerr << "Attempted to write to ROM" << std::endl;
        }
    }
    if (addr >= 0xD000 && addr <= 0xDFFF) {
        if ((dataRegister & 0b100) == 0b100) {
            handleIoWrite(addr, data);
        } else {
            std::cerr << "Attempted to write to ROM" << std::endl;
        }
    }
    ram[addr] = data;
    ramFile.seekp(addr);
    ramFile.write(reinterpret_cast<char*>(&data), 1);
}

uint8_t Bus::read(uint16_t addr) {
    // std::cout << "Reading from address: " << std::hex << addr << std::dec << "\n";
    if (addr == 0x0000) return dataDirectionRegister;
    if (addr == 0x0001) return dataRegister ;    
    if ((dataRegister  & 0b011) == 0b00) {
        return ram[addr];
    }

    if (addr >= 0xA000 && addr <= 0xBFFF) {
        if ((dataRegister & 0b011) == 0b01 || (dataRegister & 0b011) == 0b10) {
            return ram[addr];
        } else {
            return basicRom[addr - 0xA000];
        } 
    }
    if (addr >= 0xE000 && addr <= 0xFFFF) {
        if ((dataRegister & 0b011) == 0b01) {
            return ram[addr];
        } else {
            return kernelRom[addr - 0xE000];
        }
    }
    if (addr >= 0xD000 && addr <= 0xDFFF) {
        if ((dataRegister & 0b100) == 0b100) {
            return handleIoRead(addr);
        } else {
            return charRom[addr - 0xD000];
        }
    }
    return ram[addr];
}

void Bus::writeWord(uint16_t addr, uint16_t data) {
    write(addr, data & 0xFF);
    write(addr + 1, data >> 8);
}

uint16_t Bus::readWord(uint16_t addr) {
    return read(addr) | (read(addr + 1) << 8);
}

void Bus::writeBytes(uint16_t addr, const uint8_t *data, uint16_t size) {
    for (int i = 0; i < size; i++) {
        ram[addr + i] = data[i];
    }
}

uint8_t Bus::handleIoRead(uint16_t addr) {
    if (addr >= 0xD000 && addr < 0xD400) return vic->read(addr);
    if (addr >= 0xDC00 && addr < 0xDD00) return cia1->read(addr);
    return 0;
}

void Bus::handleIoWrite(uint16_t addr, uint8_t data) {
    if (addr >= 0xD000 && addr < 0xD400) vic->write(addr, data);
    if (addr >= 0xDC00 && addr < 0xDD00) cia1->write(addr, data);
}

void Bus::loadC64rom(const char *filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    file.read(reinterpret_cast<char*>(basicRom), 0x2000);
    file.read(reinterpret_cast<char*>(kernelRom), 0x2000);

    file.close();
}

void Bus::loadCharacterRom(const char *filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return;
    }

    file.read(reinterpret_cast<char*>(charRom), 0x1000);

    file.close();
}
