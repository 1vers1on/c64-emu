#include <bus.hpp>

void Bus::writeWord(uint16_t addr, uint16_t data) {
    write(addr, data & 0xFF);
    write(addr + 1, data >> 8);
}

uint16_t Bus::readWord(uint16_t addr) {
    return read(addr) | (read(addr + 1) << 8);
}

void Bus::writeBytes(uint16_t addr, const uint8_t* data, uint16_t size) {
    for(int i = 0; i < size; i++) {
        write(addr + i, data[i]);
    }
}
