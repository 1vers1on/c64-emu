#include <cia1.hpp>
#include <iostream>

CIA1::CIA1() {
    lastCycle = 4;
    for (int i = 0; i < 0x10; i++) {
        registers[i] = 0x00;
    }
}

CIA1::~CIA1() {
}

void CIA1::write(uint16_t addr, uint8_t data) {
    std::cout << "CIA1 write to address: " << std::hex << addr << " with data: " << static_cast<int>(data) << std::dec << "\n";
    addr &= 0x0F;
    registers[addr] = data;

    switch (addr) {
        case 4: 
            timerAReload = (timerAReload & 0xFF00) | data;
            timerA = timerAReload;
            break;
        case 5: 
            timerAReload = (timerAReload & 0x00FF) | (data << 8);
            timerA = timerAReload;
            break;
        case 0xD: // interrupt control
            if (data & 0x80) registers[0xD] |= (data & 0x7F); // enable
            else registers[0xD] &= ~(data & 0x7F); // disable
            break;
        case 0xE: // control register A

            if (data & 0x10) timerARunning = true; // start timer A
            else timerARunning = false; // stop timer
            break;
    }
}

uint8_t CIA1::read(uint16_t addr) {
    addr &= 0x0F;
    if (addr == 4) return timerA & 0xFF; // return low byte
    if (addr == 5) return (timerA >> 8) & 0xFF; // return high byte
    return registers[addr]; // return stored value
}

void CIA1::updateTimers(size_t cycle) {
    size_t delta = cycle - lastCycle;
    lastCycle = cycle;
    if (timerARunning) {
        // std::cout << "CIA1 timer A running" << "\n";
        if (delta >= timerA) {
            timerA = 0;
            if (registers[0xD] & 0x01) triggerInterrupt(0x01); // timer A interrupt
        } else {
            timerA -= delta;
        }
    }
}

void CIA1::triggerInterrupt(uint8_t interruptType) {
    // if (interruptType == 0x01) {
    //     registers[0xD] |= 0x01; // set timer A interrupt flag
    // }
    // if (interruptType == 0x02) {
    //     registers[0xD] |= 0x02; // set timer B interrupt flag
    // }
    // if (interruptType == 0x04) {
    //     registers[0xD] |= 0x04; // set serial interrupt flag
    // }
}
