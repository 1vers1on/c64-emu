#pragma once

#include <cstdint>
#include <cstddef>
#include <bus.hpp>
#include <serial_bus.hpp>
#include <functional>
#include <tuple>

#define PORTA 0
#define PORTB 1
#define DIRECTION_REGISTER_A 2
#define DIRECTION_REGISTER_B 3
#define TIMER_A_LOW 4
#define TIMER_A_HIGH 5
#define TIMER_B_LOW 6
#define TIMER_B_HIGH 7
#define TIME_OF_DAY_TENTHS 8
#define TIME_OF_DAY_SECONDS 9
#define TIME_OF_DAY_MINUTES 10
#define TIME_OF_DAY_HOURS 11
#define SERIAL_SHIFT_REGISTER 12
#define INTERRUPT_CONTROL_REGISTER 13
#define TIMER_A_CONTROL_REGISTER 14
#define TIMER_B_CONTROL_REGISTER 15

class Bus;
class CPU;

class CIA2 {
public:
    CIA2(Bus* bus, SerialBus* serialBus);
    ~CIA2();

    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr);

    void setCpu(CPU *cpu) { this->cpu = cpu; }

    void tick();

    void setDataSerial(bool dataIn);
    void setClockSerial(bool clockIn);

private:
    void triggerNMI(uint8_t interruptType);
    void clearNMI(uint8_t interruptType);
    
    uint32_t lastFrameCount;
    uint8_t tenthsSeconds;
    uint8_t singleSeconds;
    uint8_t tensSeconds;
    uint8_t singleMinutes;
    uint8_t tensMinutes;
    uint8_t singleHours;
    uint8_t tensHours;
    bool PM;
    
    uint16_t timerA;
    uint16_t timerAReload;
    uint16_t timerB;
    uint16_t timerBReload;
    size_t lastCycle;
    CPU *cpu;
    Bus *bus;
    SerialBus* serialBus;
    uint8_t registers[0x10];
    // uint8_t portA;
    // uint8_t portB;
    // uint8_t dataDirectionRegisterA;
    // uint8_t dataDirectionRegisterB;
    // uint8_t timerAlow;
    // uint8_t timerAhigh;
    // uint8_t timerBlow;
    // uint8_t timerBhigh;
    // uint8_t timeOfDayTenths;
    // uint8_t timeOfDaySeconds;
    // uint8_t timeOfDayMinutes;
    // uint8_t timeOfDayHours;
    // uint8_t serialShiftRegister;
    // uint8_t interruptControlRegister;
    // uint8_t timerAControlRegister;
    // uint8_t timerBControlRegister;
    bool clockIn;
    bool dataIn;
    std::function<void(bool, bool, bool)> iecBusCallback;
    std::function<std::pair<bool, bool>()> iecBusReadCallback;
};
