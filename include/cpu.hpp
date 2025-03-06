#pragma once

#include <cstdint>
#include <cstddef>
#include <bus.hpp>
#include <array>
#include <functional>
#include <tuple>
// https://www.nesdev.org/6502_cpu.txt
// https://www.oxyron.de/html/opcodes02.html
// https://www.nesdev.org/wiki/Instruction_reference
// https://www.nesdev.org/obelisk-6502-guide/reference.html
// https://www.nesdev.org/undocumented_opcodes.txt

class Bus;

enum class AddressingMode {
    IMMEDIATE,
    ZERO_PAGE,
    ZERO_PAGE_X,
    ZERO_PAGE_Y,
    ABSOLUTE,
    ABSOLUTE_X,
    ABSOLUTE_Y,
    INDIRECT,
    INDIRECT_X,
    INDIRECT_Y,
    ACCUMULATOR,
    RELATIVE,
    IMPLIED
};

#define SET_FLAG(flag) P |= flag
#define CLEAR_FLAG(flag) P &= ~flag
#define GET_FLAG(flag) (P & flag)

#define CARRY_FLAG 0b00000001
#define ZERO_FLAG 0b00000010
#define INTERRUPT_DISABLE_FLAG 0b00000100
#define DECIMAL_MODE_FLAG 0b00001000
#define BREAK_FLAG 0b00010000
#define UNUSED_FLAG 0b00100000
#define OVERFLOW_FLAG 0b01000000
#define NEGATIVE_FLAG 0b10000000

class CPU {
public:
    CPU(Bus* bus);
    ~CPU();
    
    void powerOn();
    void reset();

    void executeOnce();

    Bus* bus;

    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t SP;
    uint8_t P;

    uint16_t PC;

    size_t cycles;

    uint8_t fetch();
    uint16_t fetchWord();
    uint16_t getAddress(AddressingMode mode);

    uint8_t getCurrentOpcode() const {
        return currentOpcode;
    }

    void pushByte(uint8_t data);
    void pushWord(uint16_t data);

    void triggerIRQ();
    void triggerNMI();

    uint8_t popByte();
    uint16_t popWord();
    void stepCycles(size_t cycles);
    void stallCycles(size_t cycles);

    void setCycleCallback(std::function<void()> callback) {
        cycleCallback = callback;
    }

private:
    size_t lastCycles;
    uint8_t currentOpcode;

    int loops;

    std::array<std::tuple<std::function<void(CPU *, AddressingMode)>, AddressingMode>, 256> instructions;

    std::function<void()> cycleCallback;

    bool irqPending = false;
    bool nmiPending = false;
};
