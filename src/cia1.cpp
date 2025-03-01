#include <cia1.hpp>
#include <cpu.hpp>
#include <iostream>
#include <bitset>

CIA1::CIA1(Bus* bus) {
    this->bus = bus;
    for (int i = 0; i < 0x10; i++) {
        registers[i] = 0x00;
    }
}

CIA1::~CIA1() {
}

inline uint8_t BCDToDecimal(uint8_t bcd) {
    return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0x0F);
}

inline uint8_t decimalToBCD(uint8_t decimal) {
    return ((decimal / 10) << 4) | (decimal % 10);
}

void CIA1::write(uint16_t addr, uint8_t data) {
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

    if (addr == TIMER_A_CONTROL_REGISTER) {
        if (data & 0b00010000) {
            timerA = timerAReload;
        }
    }

    if (addr == TIMER_B_CONTROL_REGISTER) {
        if (data & 0b00010000) {
            timerB = timerBReload;
        }
    }

    if (addr == TIME_OF_DAY_TENTHS) {
        tenthsSeconds = BCDToDecimal(data);
    }

    if (addr == TIME_OF_DAY_SECONDS) {
        singleSeconds = BCDToDecimal(data & 0x0F);
        tensSeconds = BCDToDecimal((data & 0xF0) >> 4);
    }

    if (addr == TIME_OF_DAY_MINUTES) {
        singleMinutes = BCDToDecimal(data & 0x0F);
        tensMinutes = BCDToDecimal((data & 0xF0) >> 4);
    }

    if (addr == TIME_OF_DAY_HOURS) {
        singleHours = BCDToDecimal(data & 0x0F);
        tensHours = BCDToDecimal((data & 0x30) >> 4);
        PM = data & 0x20;
    }

    registers[addr] = data;
}

uint8_t CIA1::read(uint16_t addr) {
    // std::cout << "CIA1 read from address: " << std::hex << addr << std::dec << std::endl;
    addr &= 0x0F;
    if (addr == PORTB) {
        return bus->input->readKeyMatrix(registers[PORTA]);
    }
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
    if (addr == TIME_OF_DAY_TENTHS) {
        return decimalToBCD(tenthsSeconds);
    }
    if (addr == TIME_OF_DAY_SECONDS) {
        return decimalToBCD(singleSeconds) | (decimalToBCD(tensSeconds) << 4);
    }
    if (addr == TIME_OF_DAY_MINUTES) {
        return decimalToBCD(singleMinutes) | (decimalToBCD(tensMinutes) << 4);
    }
    if (addr == TIME_OF_DAY_HOURS) {
        return decimalToBCD(singleHours) | (decimalToBCD(tensHours) << 4) | (PM << 5);
    }
    return registers[addr];
}

void CIA1::tick() {
    if (lastFrameCount != bus->vic->getFrameCount()) {
        lastFrameCount = bus->vic->getFrameCount();
        if ((lastFrameCount % 5) == 0) {
            tenthsSeconds += 1;
            if (tenthsSeconds == 10) {
                tenthsSeconds = 0;
                singleSeconds += 1;
                if (singleSeconds == 10) {
                    singleSeconds = 0;
                    tensSeconds += 1;
                    if (tensSeconds == 6) {
                        tensSeconds = 0;
                        singleMinutes += 1;
                        if (singleMinutes == 10) {
                            singleMinutes = 0;
                            tensMinutes += 1;
                            if (tensMinutes == 6) {
                                tensMinutes = 0;
                                singleHours += 1;
                                if (singleHours == 10) {
                                    singleHours = 0;
                                    tensHours += 1;
                                    if (tensHours == 2) {
                                        tensHours = 0;
                                        PM = !PM;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (registers[TIMER_A_CONTROL_REGISTER] & 0x01) {
        timerA -= 1;
        if (timerA == 0) {
            triggerInterrupt(0);

            if (!(registers[TIMER_A_CONTROL_REGISTER] & 0b1000)) {
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

void CIA1::triggerInterrupt(uint8_t interruptType) {
    registers[INTERRUPT_CONTROL_REGISTER] |= (1 << (interruptType));
    cpu->triggerIRQ();
}
