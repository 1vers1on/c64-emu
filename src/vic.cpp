#include <vic.hpp>

#include <iostream>

VIC::VIC() {
    for (int i = 0; i < 0x30; i++) {
        registers[i] = 0x00;
    }
}

VIC::~VIC() {
}

uint8_t VIC::read(uint16_t addr) {
    std::cout << "VIC read from address: " << std::hex << addr << std::dec << "\n";
    addr &= 0x2F;
    if (addr == 0x12) { // raster line
        return rasterLine & 0xFF;
    }
    return registers[addr];
}

void VIC::write(uint16_t addr, uint8_t value) {
    std::cout << "VIC write to address: " << std::hex << addr << " with data: " << static_cast<int>(value) << std::dec << "\n";
    addr &= 0x2F;
    if (addr == 0x19) { // icr
        registers[0x19] &= ~value; // clear interrupt
    }
    registers[addr] = value;
}

void VIC::tick(size_t cycles) {
    cycleCounter += cycles;
    if (cycleCounter >= 63) {
        cycleCounter -= 63;
        rasterLine++;
        if (rasterLine == 312) {
            rasterLine = 0;
        }
    }
    if (rasterLine == registers[0x12]) {
        registers[0x1A] |= 0x02; // set interrupt
        setIrq(0);
    }
}

void VIC::setIrq(uint8_t bit) {
    registers[0x19] |= (1 << bit); // set interrupt
}

void VIC::checkInterrupts() {
    if (registers[0x19] & registers[0x1A]) {
        registers[0x19] |= 0x80; // set interrupt
        irqPending = true;
    } else {
        registers[0x19] &= 0x7F;
        irqPending = false;
    }
}
