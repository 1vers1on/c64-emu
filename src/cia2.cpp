#include <serial_bus.hpp>
#include <cia2.hpp>
#include <cpu.hpp>
#include <cstdint>
#include <iostream>
#include <bitset>
#

CIA2::CIA2(Bus* bus, SerialBus* serial) {
    this->bus = bus;
    for (int i = 0; i < 0x10; i++) {
        registers[i] = 0x00;
    }

    this->serialBus = serial;
}

CIA2::~CIA2() {
}

inline uint8_t BCDToDecimal(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

inline uint8_t decimalToBCD(uint8_t decimal) {
    return ((decimal / 10) << 4) | (decimal % 10);
}

void CIA2::setDataSerial(bool dataIn) {
    uint8_t serialData = registers[PORTA] & 0b11000111;
    serialData |= (dataIn << 4);
    registers[PORTA] = serialData;
}

void CIA2::setClockSerial(bool clockIn) {
    uint8_t serialData = registers[PORTA] & 0b11000111;
    serialData |= (clockIn << 3);
    registers[PORTA] = serialData;
}

void CIA2::write(uint16_t addr, uint8_t data) {
    std::cout << "CIA2 write to address: " << std::hex << addr 
              << " with data: " << std::bitset<8>(data) << std::dec << std::endl;
    addr &= 0x0F;
    uint8_t oldRegister = registers[addr];
    registers[addr] = data;
    switch (addr) {
        case PORTA: {
            // set bank address based on lower 2 bits of previous port a value
            switch (registers[PORTA] & 0b11) {

                case 0b11:
                    bus->vic->bankAddress = 0x0000;
                    break;
                case 0b10:
                    bus->vic->bankAddress = 0x4000;
                    break;
                case 0b01:
                    bus->vic->bankAddress = 0x8000;
                    break;
                case 0b00:
                    bus->vic->bankAddress = 0xC000;
                    break;
            }
            // print it as a bitset
            std::cout << "Port A: " << std::bitset<8>(registers[PORTA]) << std::endl;
            uint8_t serialData = (registers[PORTA] & ~0b11000111);
            serialData &= registers[DIRECTION_REGISTER_A];
            serialData |= (oldRegister & ~registers[DIRECTION_REGISTER_A]);
            serialData >>= 3;
            bool atnFlagOut = serialData & 0x01;
            bool clockFlagOut = serialData & 0x02;
            bool dataFlagOut = serialData & 0x04;
            std::cout << "ATN: " << atnFlagOut << " CLK: " << clockFlagOut << " DATA: " << dataFlagOut << std::endl;
            serialBus->CIAWrite({dataFlagOut, clockFlagOut, atnFlagOut});
            break;
        }
        case TIMER_A_LOW:
            timerAReload = (timerAReload & 0xFF00) | data;
            timerA = timerAReload;
            break;
        case TIMER_A_HIGH:
            timerAReload = (timerAReload & 0x00FF) | (data << 8);
            timerA = timerAReload;
            break;
        case TIMER_B_LOW:
            timerBReload = (timerBReload & 0xFF00) | data;
            timerB = timerBReload;
            break;
        case TIMER_B_HIGH:
            timerBReload = (timerBReload & 0x00FF) | (data << 8);
            timerB = timerBReload;
            break;
        case TIME_OF_DAY_TENTHS:
            tenthsSeconds = BCDToDecimal(data);
            break;
        case TIME_OF_DAY_SECONDS:
            singleSeconds = BCDToDecimal(data & 0x0F);
            tensSeconds = BCDToDecimal(data >> 4);
            break;
        case TIME_OF_DAY_MINUTES:
            singleMinutes = BCDToDecimal(data & 0x0F);
            tensMinutes = BCDToDecimal(data >> 4);
            break;
        case TIME_OF_DAY_HOURS:
            singleHours = BCDToDecimal(data & 0x0F);
            tensHours = BCDToDecimal((data & 0x30) >> 4);
            PM = data & 0x20;
            break;
        default:
            break;
    }
}



// WASM Module Loaded
// Starting emulator...
// Starting emulator
// 1
// CIA2 write to address: dd0d with data: 7f
// CIA2 write to address: dd0e with data: 8
// CIA2 write to address: dd0f with data: 8
// CIA2 write to address: dd03 with data: 0
// SID write to address: 18 with data: 0
// CIA2 write to address: dd00 with data: 7
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// CIA2 write to address: dd02 with data: 3f
// CIA2 write to address: dd00 with data: 17
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// 208862
// 226590
// 226043
// 226928
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// Attempted to write to ROM
// 228112
// CIA2 write to address: dd00 with data: 17
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// 173561
// 224231
// 213299
// 206875
// 218665
// 209975
// 198810
// 214081
// 216113
// 212499
// 221134
// 223049
// 215354
// 216620
// 218782
// 218157
// 216163
// 215928
// 211712
// 222560
// 218185
// 217508
// 222226
// 214701
// 214813
// 214409
// 217540
// 214571
// 215410
// 217530
// 216533
// 207913
// 200749
// 209746
// 213070
// 218192
// 209062
// 215021
// 208597
// CIA2 write to address: dd00 with data: 7
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// CIA2 write to address: dd00 with data: f
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// CIA2 write to address: dd00 with data: 17
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// CIA2 write to address: dd00 with data: 7
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// CIA2 write to address: dd00 with data: 7
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// CIA2 write to address: dd00 with data: 7
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0
// IEC Bus write: 0 0 0

uint8_t CIA2::read(uint16_t addr) {
    addr &= 0x0F;
    std::cout << "CIA2 read from address: " << std::hex << addr << std::dec << std::endl;

    switch (addr) {
        case PORTA: {
            uint8_t out = registers[PORTA];
            out &= 0b00111111;
            SerialPortState serialState = serialBus->CIARead();
            out |= (serialState.clockLine << 6) | (serialState.dataLine << 7);

            return out;
        }
            
        case TIMER_A_LOW:
            return timerA & 0xFF;
        case TIMER_A_HIGH:
            return timerA >> 8;
        case TIMER_B_LOW:
            return timerB & 0xFF;
        case TIMER_B_HIGH:
            return timerB >> 8;
        case TIME_OF_DAY_TENTHS:
            return decimalToBCD(tenthsSeconds);
        case TIME_OF_DAY_SECONDS:
            return decimalToBCD(singleSeconds) | (decimalToBCD(tensSeconds) << 4);
        case TIME_OF_DAY_MINUTES:
            return decimalToBCD(singleMinutes) | (decimalToBCD(tensMinutes) << 4);
        case TIME_OF_DAY_HOURS:
            return decimalToBCD(singleHours) | (decimalToBCD(tensHours) << 4) | (PM << 5);
        default:
            return registers[addr];
    }
}

void CIA2::tick() {
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
        if (--timerA == 0) {
            if (registers[INTERRUPT_CONTROL_REGISTER] & 0x01) {
                triggerNMI(0);
            }

            if (!(registers[TIMER_A_CONTROL_REGISTER] & 0b1000)) {
                timerA = timerAReload;
            } else {
                registers[TIMER_A_CONTROL_REGISTER] &= 0b11111110;
            }
        }
    }

    if (registers[TIMER_B_CONTROL_REGISTER] & 0x01) {
        if (--timerB == 0) {
            if (registers[INTERRUPT_CONTROL_REGISTER] & 0x02) {
                triggerNMI(1);
            }

            if (!(registers[TIMER_B_CONTROL_REGISTER] & 0b1000)) {
                timerB = timerBReload;
            } else {
                registers[TIMER_B_CONTROL_REGISTER] &= 0b11111110;
            }
        }
    }
}

void CIA2::triggerNMI(uint8_t interruptType) {
    registers[INTERRUPT_CONTROL_REGISTER] |= (1 << (interruptType));
    registers[INTERRUPT_CONTROL_REGISTER] |= 0x80;
    cpu->triggerNMI();
}
