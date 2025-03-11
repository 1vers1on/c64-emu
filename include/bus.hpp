#pragma once

#include <cstdint>

class Bus {
public:
    Bus() {

    }
    
    ~Bus() {

    }

    virtual void write(uint16_t addr, uint8_t data) = 0;
    virtual uint8_t read(uint16_t addr) = 0;

    void writeWord(uint16_t addr, uint16_t data);
    uint16_t readWord(uint16_t addr);

    void writeBytes(uint16_t addr, const uint8_t *data, uint16_t size);
};
