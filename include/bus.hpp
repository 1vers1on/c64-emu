#pragma once

#include <cstdint>
#include <vector>

class Bus {
public:
    Bus();
    ~Bus();

    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);

    void writeWord(uint16_t addr, uint16_t data);
    uint16_t readWord(uint16_t addr);

    void writeBytes(uint16_t addr, const uint8_t *data, uint16_t size);

private:
    uint8_t ram[0xFFFF];
};
