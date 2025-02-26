#include <cia2.hpp>
#include <cpu.hpp>
#include <iostream>

CIA2::CIA2() {
    for (int i = 0; i < 0x10; i++) {
        registers[i] = 0x00;
    }
}

CIA2::~CIA2() {
}

void CIA2::write(uint16_t addr, uint8_t data) {
    std::cout << "CIA2 write to address: " << std::hex << addr << " with data: " << static_cast<int>(data) << std::dec << std::endl;
    addr &= 0x0F;
    if (addr == TIMER_A_LOW) {
        timerAReload = (timerAReload & 0xFF00) | data;
        timerA = timerAReload;
    }
    if (addr == TIMER_A_HIGH) {
        timerAReload = (timerAReload & 0x00FF) | (data << 8);
        timerA = timerAReload;
    }
    if (addr == TIMER_B_LOW) {
        timerBReload = (timerBReload & 0xFF00) | data;
        timerB = timerBReload;
    }
    if (addr == TIMER_B_HIGH) {
        timerBReload = (timerBReload & 0x00FF) | (data << 8);
        timerB = timerBReload;
    }
    registers[addr] = data;
}

uint8_t CIA2::read(uint16_t addr) {
    std::cout << "CIA2 read from address: " << std::hex << addr << std::dec << std::endl;
    addr &= 0x0F;
    if (addr == TIMER_A_LOW) {
        return timerA & 0xFF;
    }
    if (addr == TIMER_A_HIGH) {
        return timerA >> 8;
    }
    if (addr == TIMER_B_LOW) {
        return timerB & 0xFF;
    }
    if (addr == TIMER_B_HIGH) {
        return timerB >> 8;
    }
    return registers[addr];
}

void CIA2::tick() {
    if (registers[TIMER_A_CONTROL_REGISTER] & 0x01) {
        timerA -= 1;
        if (timerA == 0) {
            if (registers[INTERRUPT_CONTROL_REGISTER] & 0x01) {
                triggerInterrupt(0);
            }

            if (!(registers[TIMER_B_CONTROL_REGISTER] & 0b1000)) {
                timerA = timerAReload;
            } else {
                registers[TIMER_A_CONTROL_REGISTER] &= 0b11111110;
            }
        }
    }

    if (registers[TIMER_B_CONTROL_REGISTER] & 0x01) {
        timerB -= 1;
        if (timerB == 0) {
            if (registers[INTERRUPT_CONTROL_REGISTER] & 0x02) {
                triggerInterrupt(1);
            }

            if (!(registers[TIMER_B_CONTROL_REGISTER] & 0b1000)) {
                timerB = timerBReload;
            } else {
                registers[TIMER_B_CONTROL_REGISTER] &= 0b11111110;
            }
        }
    }
}

void CIA2::triggerInterrupt(uint8_t interruptType) {
    registers[INTERRUPT_CONTROL_REGISTER] |= (1 << (interruptType));
    cpu->triggerIrq();
}
