#include <cpu.hpp>
#include <cstddef>
#include <sys/types.h>
#include <array>
#include <stdexcept>
#include <iostream>

static void unkownInstruction(CPU *cpu, AddressingMode mode) {
    std::cout << "opcode: " << std::hex << static_cast<int>(cpu->getCurrentOpcode()) << std::dec << "\n";
    throw std::runtime_error("Unknown instruction");
}

static void JMP(CPU *cpu, AddressingMode mode) {
    cpu->PC = cpu->getAddress(mode);
}

static void LDX(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->X = cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->X == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->X & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void LDA(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->A = cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void LDY(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->Y = cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->Y == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->Y & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void STX(CPU *cpu, AddressingMode mode) { // this fucks up the cycles dont know why but i think my known good logs are wrong.
    uint16_t address = cpu->getAddress(mode);
    cpu->bus->write(address, cpu->X);
    cpu->stepCycles(1);
}

static void STY(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->bus->write(address, cpu->Y);
    cpu->stepCycles(1);
}

static void JSR(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->pushWord(cpu->PC - 1);
    cpu->PC = address;
    cpu->stepCycles(1);
}

static void RTS(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->popWord();
    cpu->PC = address + 1;
    cpu->stepCycles(3);
}

static void NOP(CPU *cpu, AddressingMode mode) {
    cpu->stepCycles(1);
}

static void SEC(CPU *cpu, AddressingMode mode) {
    cpu->P |= CARRY_FLAG;
    cpu->stepCycles(1);
}

static void CLC(CPU *cpu, AddressingMode mode) {
    cpu->P &= ~CARRY_FLAG;
    cpu->stepCycles(1);
}

static void SEI(CPU *cpu, AddressingMode mode) {
    cpu->P |= INTERRUPT_DISABLE_FLAG;
    cpu->stepCycles(1);
}

static void CLI(CPU *cpu, AddressingMode mode) {
    cpu->P &= ~INTERRUPT_DISABLE_FLAG;
    cpu->stepCycles(1);
}

static void SED(CPU *cpu, AddressingMode mode) {
    cpu->P |= DECIMAL_MODE_FLAG;
    cpu->stepCycles(1);
}

static void CLD(CPU *cpu, AddressingMode mode) {
    cpu->P &= ~DECIMAL_MODE_FLAG;
    cpu->stepCycles(1);
}

static void CLV(CPU *cpu, AddressingMode mode) {
    cpu->P &= ~OVERFLOW_FLAG;
    cpu->stepCycles(1);
}

static void BCS(CPU *cpu, AddressingMode mode) {
    int8_t offset = cpu->fetch();
    if (cpu->P & CARRY_FLAG) {
        cpu->stepCycles(1);
        if ((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset; 
    }
    cpu->stepCycles(1);
}

static void BCC(CPU *cpu, AddressingMode mode) {
    int8_t offset = cpu->fetch();
    if (!(cpu->P & CARRY_FLAG)) {
        cpu->stepCycles(1);
        if ((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset; 
    }
    cpu->stepCycles(1);
}

static void BEQ(CPU *cpu, AddressingMode mode) {
    int8_t offset = cpu->fetch();
    if (cpu->P & ZERO_FLAG) {
        cpu->stepCycles(1);
        if ((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset; 
    }
    cpu->stepCycles(1);
}

static void BNE(CPU *cpu, AddressingMode mode) {
    int8_t offset = cpu->fetch();
    if (!(cpu->P & ZERO_FLAG)) {
        cpu->stepCycles(1);
        if ((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset; 
    }
    cpu->stepCycles(1);
}

static void BVS(CPU *cpu, AddressingMode mode) {
    int8_t offset = cpu->fetch();
    if (cpu->P & OVERFLOW_FLAG) {
        cpu->stepCycles(1);
        if ((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset; 
    }
    cpu->stepCycles(1);
}

static void BVC(CPU *cpu, AddressingMode mode) {
    int8_t offset = cpu->fetch();
    if (!(cpu->P & OVERFLOW_FLAG)) {
        cpu->stepCycles(1);
        if ((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset; 
    }
    cpu->stepCycles(1);
}

static void BPL(CPU *cpu, AddressingMode mode) {
    int8_t offset = cpu->fetch();
    if (!(cpu->P & NEGATIVE_FLAG)) {
        cpu->stepCycles(1);
        if ((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset; 
    }
    cpu->stepCycles(1);
}

static void BMI(CPU *cpu, AddressingMode mode) {
    int8_t offset = cpu->fetch();
    if (cpu->P & NEGATIVE_FLAG) {
        cpu->stepCycles(1);
        if ((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset; 
    }
    cpu->stepCycles(1);
}

static void STA(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->bus->write(address, cpu->A);
    cpu->stepCycles(1);
}

static void BIT(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~OVERFLOW_FLAG;
    if ((cpu->A & data) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if (data & 0x40) {
        cpu->P |= OVERFLOW_FLAG;
    }
    cpu->stepCycles(1);
}

static void PHP(CPU *cpu, AddressingMode mode) {
    cpu->pushByte(cpu->P | BREAK_FLAG | UNUSED_FLAG);
    cpu->stepCycles(1);
}

static void PLP(CPU *cpu, AddressingMode mode) {
    cpu->P = (cpu->popByte() & 0xEF) | (cpu->P & 0x10) | 0x20;
    cpu->stepCycles(1);
}

static void PHA(CPU *cpu, AddressingMode mode) {
    cpu->pushByte(cpu->A);
    cpu->stepCycles(1);
}

static void PLA(CPU *cpu, AddressingMode mode) {
    cpu->A = cpu->popByte();
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(2);
}

// static void RTI(CPU *cpu, AddressingMode mode) {
//     cpu->P = (cpu->popByte() & 0xEF) | (cpu->P & 0x10) | 0x20;
//     cpu->PC = cpu->popWord();
//     cpu->stepCycles(1);
// }

static void RTI(CPU *cpu, AddressingMode mode) {
    cpu->P = cpu->popByte();  // Pop status register
    cpu->P |= 0x20;  // Ensure unused bit is set
    cpu->PC = cpu->popWord();  // Restore PC
}


static void AND(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->A &= cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void CMP(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint16_t result = cpu->A - data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    if ((result & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if (cpu->A >= data) {
        cpu->P |= CARRY_FLAG;
    }
    cpu->stepCycles(1);
}

static void CPX(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint16_t result = cpu->X - data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    if (result == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if (cpu->X >= data) {
        cpu->P |= CARRY_FLAG;
    }
    cpu->stepCycles(1);
}

static void CPY(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint16_t result = cpu->Y - data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    if (result == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if (cpu->Y >= data) {
        cpu->P |= CARRY_FLAG;
    }
    cpu->stepCycles(1);
}

static void ORA(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->A |= cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void EOR(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->A ^= cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

// we will implement decimal mode later :3
static void ADC(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint16_t result = cpu->A + data + (cpu->P & CARRY_FLAG);
    
    if (cpu->P & DECIMAL_MODE_FLAG) {
        if ((cpu->A & 0x0F) + (data & 0x0F) + (cpu->P & CARRY_FLAG) > 0x09) {
            result += 0x06;
        }
        if (result > 0x99) {
            result += 0x60;
        }
    }

    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~OVERFLOW_FLAG;
    if ((result & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if (result > 0xFF) {
        cpu->P |= CARRY_FLAG;
    }
    if ((cpu->A ^ result) & (data ^ result) & 0x80) {
        cpu->P |= OVERFLOW_FLAG;
    }
    cpu->A = result;
    cpu->stepCycles(1);
}

static void SBC(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint16_t result = cpu->A - data - (1 - (cpu->P & CARRY_FLAG));

    if (cpu->P & DECIMAL_MODE_FLAG) {
        if ((cpu->A & 0x0F) < (data & 0x0F) + (1 - (cpu->P & CARRY_FLAG))) {
            result -= 0x06;
        }
        if (result & 0x100) {
            result -= 0x60;
        }
    }

    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~OVERFLOW_FLAG;
    if ((result & 0xff) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if (result < 0x100) {
        cpu->P |= CARRY_FLAG;
    }
    if (((cpu->A ^ result) & (cpu->A ^ data) & 0x80) != 0) {
        cpu->P |= OVERFLOW_FLAG;
    }
    cpu->A = result;
    cpu->stepCycles(1);
}

static void DEC(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address) - 1;
    cpu->bus->write(address, data);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void DEY(CPU *cpu, AddressingMode mode) {
    cpu->Y -= 1;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->Y == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->Y & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void DEX(CPU *cpu, AddressingMode mode) {
    cpu->X -= 1;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->X == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->X & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void INC(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address) + 1;
    cpu->bus->write(address, data);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void INY(CPU *cpu, AddressingMode mode) {
    cpu->Y += 1;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->Y == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->Y & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void INX(CPU *cpu, AddressingMode mode) {
    cpu->X += 1;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->X == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->X & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void TAY(CPU *cpu, AddressingMode mode) {
    cpu->Y = cpu->A;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->Y == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->Y & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void TAX(CPU *cpu, AddressingMode mode) {
    cpu->X = cpu->A;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->X == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->X & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void TSX(CPU *cpu, AddressingMode mode) {
    cpu->X = cpu->SP;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->X == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->X & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void TYA(CPU *cpu, AddressingMode mode) {
    cpu->A = cpu->Y;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void TXA(CPU *cpu, AddressingMode mode) {
    cpu->A = cpu->X;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void TXS(CPU *cpu, AddressingMode mode) {
    cpu->SP = cpu->X;
    cpu->stepCycles(1);
}

static void LSR(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    if (mode == AddressingMode::ACCUMULATOR) {
        cpu->P &= ~CARRY_FLAG;
        cpu->P &= ~ZERO_FLAG;
        cpu->P &= ~NEGATIVE_FLAG;
        if (cpu->A & 0x01) {
            cpu->P |= CARRY_FLAG;
        }
        cpu->A >>= 1;
        if (cpu->A == 0) {
            cpu->P |= ZERO_FLAG;
        }
        if (cpu->A & 0x80) {
            cpu->P |= NEGATIVE_FLAG;
        }
        cpu->stepCycles(1);
        return;
    }
    uint8_t data = cpu->bus->read(address);
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (data & 0x01) {
        cpu->P |= CARRY_FLAG;
    }
    data >>= 1;
    if (data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->stepCycles(1);
}

static void ASL(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    if (mode == AddressingMode::ACCUMULATOR) {
        cpu->P &= ~CARRY_FLAG;
        cpu->P &= ~ZERO_FLAG;
        cpu->P &= ~NEGATIVE_FLAG;
        if (cpu->A & 0x80) {
            cpu->P |= CARRY_FLAG;
        }
        cpu->A <<= 1;
        if (cpu->A == 0) {
            cpu->P |= ZERO_FLAG;
        }
        if (cpu->A & 0x80) {
            cpu->P |= NEGATIVE_FLAG;
        }
        cpu->stepCycles(1);
        return;
    }
    uint8_t data = cpu->bus->read(address);
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (data & 0x80) {
        cpu->P |= CARRY_FLAG;
    }
    data <<= 1;
    if (data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->stepCycles(1);
}

static void ROR(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    if (mode == AddressingMode::ACCUMULATOR) {
        uint8_t carry = cpu->P & CARRY_FLAG;
        cpu->P &= ~CARRY_FLAG;
        cpu->P &= ~ZERO_FLAG;
        cpu->P &= ~NEGATIVE_FLAG;
        if (cpu->A & 0x01) {
            cpu->P |= CARRY_FLAG;
        }
        cpu->A >>= 1;
        if (carry) {
            cpu->A |= 0x80;
        }
        if (cpu->A == 0) {
            cpu->P |= ZERO_FLAG;
        }
        if (cpu->A & 0x80) {
            cpu->P |= NEGATIVE_FLAG;
        }
        cpu->stepCycles(1);
        return;
    }
    uint8_t data = cpu->bus->read(address);
    uint8_t carry = cpu->P & CARRY_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (data & 0x01) {
        cpu->P |= CARRY_FLAG;
    }
    data >>= 1;
    if (carry) {
        data |= 0x80;
    }
    if (data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->stepCycles(1);
}

static void ROL(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    if (mode == AddressingMode::ACCUMULATOR) {
        uint8_t carry = cpu->P & CARRY_FLAG;
        cpu->P &= ~CARRY_FLAG;
        cpu->P &= ~ZERO_FLAG;
        cpu->P &= ~NEGATIVE_FLAG;
        if (cpu->A & 0x80) {
            cpu->P |= CARRY_FLAG;
        }
        cpu->A <<= 1;
        if (carry) {
            cpu->A |= 0x01;
        }
        if (cpu->A == 0) {
            cpu->P |= ZERO_FLAG;
        }
        if (cpu->A & 0x80) {
            cpu->P |= NEGATIVE_FLAG;
        }
        cpu->stepCycles(1);
        return;
    }
    uint8_t data = cpu->bus->read(address);
    uint8_t carry = cpu->P & CARRY_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (data & 0x80) {
        cpu->P |= CARRY_FLAG;
    }
    data <<= 1;
    if (carry) {
        data |= 0x01;
    }
    if (data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->stepCycles(1);
}

static void BRK(CPU *cpu, AddressingMode mode) {
    cpu->PC += 1;
    cpu->pushWord(cpu->PC);
    cpu->pushByte(cpu->P | BREAK_FLAG | UNUSED_FLAG);
    cpu->P |= INTERRUPT_DISABLE_FLAG;
    cpu->PC = cpu->bus->readWord(0xFFFE);
    cpu->stepCycles(1);
}

// beware. illegal opcodes beyond!!!
static void DOP(CPU *cpu, AddressingMode mode) {
    cpu->getAddress(mode);
    cpu->stepCycles(1);
}

static void TOP(CPU *cpu, AddressingMode mode) {
    cpu->getAddress(mode);
    cpu->stepCycles(1);
}

static void LAX(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->A = data;
    cpu->X = data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void AAX(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->A & cpu->X;
    cpu->bus->write(address, data);
    cpu->stepCycles(1);
}

static void DCP(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address) - 1;
    cpu->bus->write(address, data);
    uint16_t result = cpu->A - data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    if ((result & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if (cpu->A >= data) {
        cpu->P |= CARRY_FLAG;
    }
    cpu->stepCycles(3);
}

static void ISC(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address) + 1;
    cpu->bus->write(address, data);
    uint16_t result = cpu->A - data - (1 - (cpu->P & CARRY_FLAG));
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~OVERFLOW_FLAG;
    if ((result & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if (result < 0x100) {
        cpu->P |= CARRY_FLAG;
    }
    if (((cpu->A ^ result) & (cpu->A ^ data) & 0x80) != 0) {
        cpu->P |= OVERFLOW_FLAG;
    }
    cpu->A = result & 0xFF;
    cpu->stepCycles(3);
}

static void SLO(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (data & 0x80) {
        cpu->P |= CARRY_FLAG;
    }
    data <<= 1;
    if ((data & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->A |= data;

    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;

    if (cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(3);
}

static void RLA(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint8_t carry = cpu->P & CARRY_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (data & 0x80) {
        cpu->P |= CARRY_FLAG;
    }
    data <<= 1;
    if (carry) {
        data |= 0x01;
    }
    if (data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->A &= data;

    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;

    if (cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(3);
}

static void SRE(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (data & 0x01) {
        cpu->P |= CARRY_FLAG;
    }
    data >>= 1;
    if ((data & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->A ^= data;

    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;

    if (cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(3);
}

static void RRA(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint8_t carry = cpu->P & CARRY_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (data & 0x01) {
        cpu->P |= CARRY_FLAG;
    }
    data >>= 1;
    if (carry) {
        data |= 0x80;
    }
    if (data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    uint16_t result = cpu->A + data + (cpu->P & CARRY_FLAG);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~OVERFLOW_FLAG;
    if ((result & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if (result > 0xFF) {
        cpu->P |= CARRY_FLAG;
    }
    if ((cpu->A ^ result) & (data ^ result) & 0x80) {
        cpu->P |= OVERFLOW_FLAG;
    }
    cpu->A = result;
    cpu->stepCycles(3);
}

// probably not right but ok
static void XAA(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->A &= cpu->X & data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if (cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if (cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
}

static void KIL(CPU *cpu, AddressingMode mode) {
    cpu->PC -= 1;
    cpu->stepCycles(1);
}

// AND X register with accumulator and store result in stack pointer, then
// AND stack pointer with the high byte of the target address of the
// argument + 1. Store result in memory.
static void XAS(CPU *cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->X = cpu->A & cpu->X;
    cpu->SP = cpu->X;
    cpu->bus->write(address, cpu->SP & ((address >> 8) + 1));
    cpu->stepCycles(1);
}

CPU::CPU(Bus *bus) {
    this->bus = bus;

    instructions.fill({unkownInstruction, AddressingMode::IMPLIED, "UNK"});

    instructions[0x9B] = {XAS, AddressingMode::ABSOLUTE_Y, "XAS"};

    instructions[0x02] = {KIL, AddressingMode::IMPLIED, "KIL"};
    instructions[0x12] = {KIL, AddressingMode::IMPLIED, "KIL"};
    instructions[0x22] = {KIL, AddressingMode::IMPLIED, "KIL"};
    instructions[0x32] = {KIL, AddressingMode::IMPLIED, "KIL"};
    instructions[0x42] = {KIL, AddressingMode::IMPLIED, "KIL"};
    instructions[0x52] = {KIL, AddressingMode::IMPLIED, "KIL"};
    instructions[0x62] = {KIL, AddressingMode::IMPLIED, "KIL"};
    instructions[0x72] = {KIL, AddressingMode::IMPLIED, "KIL"};
    instructions[0x92] = {KIL, AddressingMode::IMPLIED, "KIL"};
    instructions[0xB2] = {KIL, AddressingMode::IMPLIED, "KIL"};
    instructions[0xD2] = {KIL, AddressingMode::IMPLIED, "KIL"};
    instructions[0xF2] = {KIL, AddressingMode::IMPLIED, "KIL"};

    instructions[0x00] = {BRK, AddressingMode::IMPLIED, "BRK"};

    instructions[0x67] = {RRA, AddressingMode::ZERO_PAGE, "RRA"};
    instructions[0x77] = {RRA, AddressingMode::ZERO_PAGE_X, "RRA"};
    instructions[0x6F] = {RRA, AddressingMode::ABSOLUTE, "RRA"};
    instructions[0x7F] = {RRA, AddressingMode::ABSOLUTE_X, "RRA"};
    instructions[0x7B] = {RRA, AddressingMode::ABSOLUTE_Y, "RRA"};
    instructions[0x63] = {RRA, AddressingMode::INDIRECT_X, "RRA"};
    instructions[0x73] = {RRA, AddressingMode::INDIRECT_Y, "RRA"};

    instructions[0x47] = {SRE, AddressingMode::ZERO_PAGE, "SRE"};
    instructions[0x57] = {SRE, AddressingMode::ZERO_PAGE_X, "SRE"};
    instructions[0x4F] = {SRE, AddressingMode::ABSOLUTE, "SRE"};
    instructions[0x5F] = {SRE, AddressingMode::ABSOLUTE_X, "SRE"};
    instructions[0x5B] = {SRE, AddressingMode::ABSOLUTE_Y, "SRE"};
    instructions[0x43] = {SRE, AddressingMode::INDIRECT_X, "SRE"};
    instructions[0x53] = {SRE, AddressingMode::INDIRECT_Y, "SRE"};

    instructions[0x27] = {RLA, AddressingMode::ZERO_PAGE, "RLA"};
    instructions[0x37] = {RLA, AddressingMode::ZERO_PAGE_X, "RLA"};
    instructions[0x2F] = {RLA, AddressingMode::ABSOLUTE, "RLA"};
    instructions[0x3F] = {RLA, AddressingMode::ABSOLUTE_X, "RLA"};
    instructions[0x3B] = {RLA, AddressingMode::ABSOLUTE_Y, "RLA"};
    instructions[0x23] = {RLA, AddressingMode::INDIRECT_X, "RLA"};
    instructions[0x33] = {RLA, AddressingMode::INDIRECT_Y, "RLA"};

    instructions[0x07] = {SLO, AddressingMode::ZERO_PAGE, "SLO"};
    instructions[0x17] = {SLO, AddressingMode::ZERO_PAGE_X, "SLO"};
    instructions[0x0F] = {SLO, AddressingMode::ABSOLUTE, "SLO"};
    instructions[0x1F] = {SLO, AddressingMode::ABSOLUTE_X, "SLO"};
    instructions[0x1B] = {SLO, AddressingMode::ABSOLUTE_Y, "SLO"};
    instructions[0x03] = {SLO, AddressingMode::INDIRECT_X, "SLO"};
    instructions[0x13] = {SLO, AddressingMode::INDIRECT_Y, "SLO"};

    instructions[0xE7] = {ISC, AddressingMode::ZERO_PAGE, "ISC"};
    instructions[0xF7] = {ISC, AddressingMode::ZERO_PAGE_X, "ISC"};
    instructions[0xEF] = {ISC, AddressingMode::ABSOLUTE, "ISC"};
    instructions[0xFF] = {ISC, AddressingMode::ABSOLUTE_X, "ISC"};
    instructions[0xFB] = {ISC, AddressingMode::ABSOLUTE_Y, "ISC"};
    instructions[0xE3] = {ISC, AddressingMode::INDIRECT_X, "ISC"};
    instructions[0xF3] = {ISC, AddressingMode::INDIRECT_Y, "ISC"};

    instructions[0xC7] = {DCP, AddressingMode::ZERO_PAGE, "DCP"};
    instructions[0xD7] = {DCP, AddressingMode::ZERO_PAGE_X, "DCP"};
    instructions[0xCF] = {DCP, AddressingMode::ABSOLUTE, "DCP"};
    instructions[0xDF] = {DCP, AddressingMode::ABSOLUTE_X, "DCP"};
    instructions[0xDB] = {DCP, AddressingMode::ABSOLUTE_Y, "DCP"};
    instructions[0xC3] = {DCP, AddressingMode::INDIRECT_X, "DCP"};
    instructions[0xD3] = {DCP, AddressingMode::INDIRECT_Y, "DCP"};

    instructions[0xEB] = {SBC, AddressingMode::IMMEDIATE, "SBC"};

    instructions[0x87] = {AAX, AddressingMode::ZERO_PAGE, "AAX"};
    instructions[0x97] = {AAX, AddressingMode::ZERO_PAGE_Y, "AAX"};
    instructions[0x83] = {AAX, AddressingMode::INDIRECT_X, "AAX"};
    instructions[0x8F] = {AAX, AddressingMode::ABSOLUTE, "AAX"};

    instructions[0xA7] = {LAX, AddressingMode::ZERO_PAGE, "LAX"};
    instructions[0xB7] = {LAX, AddressingMode::ZERO_PAGE_Y, "LAX"};
    instructions[0xAF] = {LAX, AddressingMode::ABSOLUTE, "LAX"};
    instructions[0xBF] = {LAX, AddressingMode::ABSOLUTE_Y, "LAX"};
    instructions[0xA3] = {LAX, AddressingMode::INDIRECT_X, "LAX"};
    instructions[0xB3] = {LAX, AddressingMode::INDIRECT_Y, "LAX"};

    instructions[0x1A] = {NOP, AddressingMode::IMPLIED, "NOP"};
    instructions[0x3A] = {NOP, AddressingMode::IMPLIED, "NOP"};
    instructions[0x5A] = {NOP, AddressingMode::IMPLIED, "NOP"};
    instructions[0x7A] = {NOP, AddressingMode::IMPLIED, "NOP"};
    instructions[0xDA] = {NOP, AddressingMode::IMPLIED, "NOP"};
    instructions[0xFA] = {NOP, AddressingMode::IMPLIED, "NOP"};

    instructions[0x0C] = {TOP, AddressingMode::ABSOLUTE, "TOP"};
    instructions[0x1C] = {TOP, AddressingMode::ABSOLUTE_X, "TOP"};
    instructions[0x3C] = {TOP, AddressingMode::ABSOLUTE_X, "TOP"};
    instructions[0x5C] = {TOP, AddressingMode::ABSOLUTE_X, "TOP"};
    instructions[0x7C] = {TOP, AddressingMode::ABSOLUTE_X, "TOP"};
    instructions[0xDC] = {TOP, AddressingMode::ABSOLUTE_X, "TOP"};
    instructions[0xFC] = {TOP, AddressingMode::ABSOLUTE_X, "TOP"};

    instructions[0x04] = {DOP, AddressingMode::ZERO_PAGE, "DOP"};
    instructions[0x14] = {DOP, AddressingMode::ZERO_PAGE_X, "DOP"};
    instructions[0x34] = {DOP, AddressingMode::ZERO_PAGE_X, "DOP"};
    instructions[0x44] = {DOP, AddressingMode::ZERO_PAGE, "DOP"};
    instructions[0x54] = {DOP, AddressingMode::ZERO_PAGE_X, "DOP"};
    instructions[0x64] = {DOP, AddressingMode::ZERO_PAGE, "DOP"};
    instructions[0x74] = {DOP, AddressingMode::ZERO_PAGE_X, "DOP"};
    instructions[0x80] = {DOP, AddressingMode::IMMEDIATE, "DOP"};
    instructions[0x82] = {DOP, AddressingMode::IMMEDIATE, "DOP"};
    instructions[0x89] = {DOP, AddressingMode::IMMEDIATE, "DOP"};
    instructions[0xC2] = {DOP, AddressingMode::IMMEDIATE, "DOP"};
    instructions[0xD4] = {DOP, AddressingMode::ZERO_PAGE_X, "DOP"};
    instructions[0xE2] = {DOP, AddressingMode::IMMEDIATE, "DOP"};
    instructions[0xF4] = {DOP, AddressingMode::ZERO_PAGE_X, "DOP"};

    instructions[0x2A] = {ROL, AddressingMode::ACCUMULATOR, "ROL"};
    instructions[0x26] = {ROL, AddressingMode::ZERO_PAGE, "ROL"};
    instructions[0x36] = {ROL, AddressingMode::ZERO_PAGE_X, "ROL"};
    instructions[0x2E] = {ROL, AddressingMode::ABSOLUTE, "ROL"};
    instructions[0x3E] = {ROL, AddressingMode::ABSOLUTE_X, "ROL"};

    instructions[0x6A] = {ROR, AddressingMode::ACCUMULATOR, "ROR"};
    instructions[0x66] = {ROR, AddressingMode::ZERO_PAGE, "ROR"};
    instructions[0x76] = {ROR, AddressingMode::ZERO_PAGE_X, "ROR"};
    instructions[0x6E] = {ROR, AddressingMode::ABSOLUTE, "ROR"};
    instructions[0x7E] = {ROR, AddressingMode::ABSOLUTE_X, "ROR"};

    instructions[0x0A] = {ASL, AddressingMode::ACCUMULATOR, "ASL"};
    instructions[0x06] = {ASL, AddressingMode::ZERO_PAGE, "ASL"};
    instructions[0x16] = {ASL, AddressingMode::ZERO_PAGE_X, "ASL"};
    instructions[0x0E] = {ASL, AddressingMode::ABSOLUTE, "ASL"};
    instructions[0x1E] = {ASL, AddressingMode::ABSOLUTE_X, "ASL"};

    instructions[0x4A] = {LSR, AddressingMode::ACCUMULATOR, "LSR"};
    instructions[0x46] = {LSR, AddressingMode::ZERO_PAGE, "LSR"};
    instructions[0x56] = {LSR, AddressingMode::ZERO_PAGE_X, "LSR"};
    instructions[0x4E] = {LSR, AddressingMode::ABSOLUTE, "LSR"};
    instructions[0x5E] = {LSR, AddressingMode::ABSOLUTE_X, "LSR"};


    instructions[0x40] = {RTI, AddressingMode::IMPLIED, "RTI"};

    instructions[0xAA] = {TAX, AddressingMode::IMPLIED, "TAX"};
    instructions[0xA8] = {TAY, AddressingMode::IMPLIED, "TAY"};
    instructions[0xBA] = {TSX, AddressingMode::IMPLIED, "TSX"};
    instructions[0x98] = {TYA, AddressingMode::IMPLIED, "TYA"};
    instructions[0x8A] = {TXA, AddressingMode::IMPLIED, "TXA"};
    instructions[0x9A] = {TXS, AddressingMode::IMPLIED, "TXS"};

    instructions[0x4C] = {JMP, AddressingMode::ABSOLUTE, "JMP"};
    instructions[0x6C] = {JMP, AddressingMode::INDIRECT, "JMP"};

    instructions[0xA2] = {LDX, AddressingMode::IMMEDIATE, "LDX"};
    instructions[0xA6] = {LDX, AddressingMode::ZERO_PAGE, "LDX"};
    instructions[0xB6] = {LDX, AddressingMode::ZERO_PAGE_Y, "LDX"};
    instructions[0xAE] = {LDX, AddressingMode::ABSOLUTE, "LDX"};
    instructions[0xBE] = {LDX, AddressingMode::ABSOLUTE_Y, "LDX"};

    instructions[0xA9] = {LDA, AddressingMode::IMMEDIATE, "LDA"};
    instructions[0xA5] = {LDA, AddressingMode::ZERO_PAGE, "LDA"};
    instructions[0xB5] = {LDA, AddressingMode::ZERO_PAGE_X, "LDA"};
    instructions[0xAD] = {LDA, AddressingMode::ABSOLUTE, "LDA"};
    instructions[0xBD] = {LDA, AddressingMode::ABSOLUTE_X, "LDA"};
    instructions[0xB9] = {LDA, AddressingMode::ABSOLUTE_Y, "LDA"};
    instructions[0xA1] = {LDA, AddressingMode::INDIRECT_X, "LDA"};
    instructions[0xB1] = {LDA, AddressingMode::INDIRECT_Y, "LDA"};

    instructions[0xA0] = {LDY, AddressingMode::IMMEDIATE, "LDY"};
    instructions[0xA4] = {LDY, AddressingMode::ZERO_PAGE, "LDY"};
    instructions[0xB4] = {LDY, AddressingMode::ZERO_PAGE_X, "LDY"};
    instructions[0xAC] = {LDY, AddressingMode::ABSOLUTE, "LDY"};
    instructions[0xBC] = {LDY, AddressingMode::ABSOLUTE_X, "LDY"};

    instructions[0x86] = {STX, AddressingMode::ZERO_PAGE, "STX"};
    instructions[0x96] = {STX, AddressingMode::ZERO_PAGE_Y, "STX"};
    instructions[0x8E] = {STX, AddressingMode::ABSOLUTE, "STX"};

    instructions[0x84] = {STY, AddressingMode::ZERO_PAGE, "STY"};
    instructions[0x94] = {STY, AddressingMode::ZERO_PAGE_X, "STY"};
    instructions[0x8C] = {STY, AddressingMode::ABSOLUTE, "STY"};

    instructions[0x20] = {JSR, AddressingMode::ABSOLUTE, "JSR"};
    instructions[0x60] = {RTS, AddressingMode::IMPLIED, "RTS"};

    instructions[0xEA] = {NOP, AddressingMode::IMPLIED, "NOP"};

    instructions[0x38] = {SEC, AddressingMode::IMPLIED, "SEC"};
    instructions[0x18] = {CLC, AddressingMode::IMPLIED, "CLC"};
    instructions[0x78] = {SEI, AddressingMode::IMPLIED, "SEI"};
    instructions[0x58] = {CLI, AddressingMode::IMPLIED, "CLI"};
    instructions[0xF8] = {SED, AddressingMode::IMPLIED, "SED"};
    instructions[0xD8] = {CLD, AddressingMode::IMPLIED, "CLD"};
    instructions[0xB8] = {CLV, AddressingMode::IMPLIED, "CLV"};

    instructions[0xB0] = {BCS, AddressingMode::RELATIVE, "BCS"};
    instructions[0x90] = {BCC, AddressingMode::RELATIVE, "BCC"};
    instructions[0xF0] = {BEQ, AddressingMode::RELATIVE, "BEQ"};
    instructions[0xD0] = {BNE, AddressingMode::RELATIVE, "BNE"};
    instructions[0x70] = {BVS, AddressingMode::RELATIVE, "BVS"};
    instructions[0x50] = {BVC, AddressingMode::RELATIVE, "BVC"};
    instructions[0x10] = {BPL, AddressingMode::RELATIVE, "BPL"};
    instructions[0x30] = {BMI, AddressingMode::RELATIVE, "BMI"};

    instructions[0x85] = {STA, AddressingMode::ZERO_PAGE, "STA"};
    instructions[0x95] = {STA, AddressingMode::ZERO_PAGE_X, "STA"};
    instructions[0x8D] = {STA, AddressingMode::ABSOLUTE, "STA"};
    instructions[0x9D] = {STA, AddressingMode::ABSOLUTE_X, "STA"};
    instructions[0x99] = {STA, AddressingMode::ABSOLUTE_Y, "STA"};
    instructions[0x81] = {STA, AddressingMode::INDIRECT_X, "STA"};
    instructions[0x91] = {STA, AddressingMode::INDIRECT_Y, "STA"};

    instructions[0x24] = {BIT, AddressingMode::ZERO_PAGE, "BIT"};
    instructions[0x2C] = {BIT, AddressingMode::ABSOLUTE, "BIT"};

    instructions[0x08] = {PHP, AddressingMode::IMPLIED, "PHP"};
    instructions[0x28] = {PLP, AddressingMode::IMPLIED, "PLP"};

    instructions[0x48] = {PHA, AddressingMode::IMPLIED, "PHA"};
    instructions[0x68] = {PLA, AddressingMode::IMPLIED, "PLA"};

    instructions[0x29] = {AND, AddressingMode::IMMEDIATE, "AND"};
    instructions[0x25] = {AND, AddressingMode::ZERO_PAGE, "AND"};
    instructions[0x35] = {AND, AddressingMode::ZERO_PAGE_X, "AND"};
    instructions[0x2D] = {AND, AddressingMode::ABSOLUTE, "AND"};
    instructions[0x3D] = {AND, AddressingMode::ABSOLUTE_X, "AND"};
    instructions[0x39] = {AND, AddressingMode::ABSOLUTE_Y, "AND"};
    instructions[0x21] = {AND, AddressingMode::INDIRECT_X, "AND"};
    instructions[0x31] = {AND, AddressingMode::INDIRECT_Y, "AND"};

    instructions[0xC9] = {CMP, AddressingMode::IMMEDIATE, "CMP"};
    instructions[0xC5] = {CMP, AddressingMode::ZERO_PAGE, "CMP"};
    instructions[0xD5] = {CMP, AddressingMode::ZERO_PAGE_X, "CMP"};
    instructions[0xCD] = {CMP, AddressingMode::ABSOLUTE, "CMP"};
    instructions[0xDD] = {CMP, AddressingMode::ABSOLUTE_X, "CMP"};
    instructions[0xD9] = {CMP, AddressingMode::ABSOLUTE_Y, "CMP"};
    instructions[0xC1] = {CMP, AddressingMode::INDIRECT_X, "CMP"};
    instructions[0xD1] = {CMP, AddressingMode::INDIRECT_Y, "CMP"};

    instructions[0xE0] = {CPX, AddressingMode::IMMEDIATE, "CPX"};
    instructions[0xE4] = {CPX, AddressingMode::ZERO_PAGE, "CPX"};
    instructions[0xEC] = {CPX, AddressingMode::ABSOLUTE, "CPX"};

    instructions[0xC0] = {CPY, AddressingMode::IMMEDIATE, "CPY"};
    instructions[0xC4] = {CPY, AddressingMode::ZERO_PAGE, "CPY"};
    instructions[0xCC] = {CPY, AddressingMode::ABSOLUTE, "CPY"};

    instructions[0x09] = {ORA, AddressingMode::IMMEDIATE, "ORA"};
    instructions[0x05] = {ORA, AddressingMode::ZERO_PAGE, "ORA"};
    instructions[0x15] = {ORA, AddressingMode::ZERO_PAGE_X, "ORA"};
    instructions[0x0D] = {ORA, AddressingMode::ABSOLUTE, "ORA"};
    instructions[0x1D] = {ORA, AddressingMode::ABSOLUTE_X, "ORA"};
    instructions[0x19] = {ORA, AddressingMode::ABSOLUTE_Y, "ORA"};
    instructions[0x01] = {ORA, AddressingMode::INDIRECT_X, "ORA"};
    instructions[0x11] = {ORA, AddressingMode::INDIRECT_Y, "ORA"};

    instructions[0x49] = {EOR, AddressingMode::IMMEDIATE, "EOR"};
    instructions[0x45] = {EOR, AddressingMode::ZERO_PAGE, "EOR"};
    instructions[0x55] = {EOR, AddressingMode::ZERO_PAGE_X, "EOR"};
    instructions[0x4D] = {EOR, AddressingMode::ABSOLUTE, "EOR"};
    instructions[0x5D] = {EOR, AddressingMode::ABSOLUTE_X, "EOR"};
    instructions[0x59] = {EOR, AddressingMode::ABSOLUTE_Y, "EOR"};
    instructions[0x41] = {EOR, AddressingMode::INDIRECT_X, "EOR"};
    instructions[0x51] = {EOR, AddressingMode::INDIRECT_Y, "EOR"};

    instructions[0x69] = {ADC, AddressingMode::IMMEDIATE, "ADC"};
    instructions[0x65] = {ADC, AddressingMode::ZERO_PAGE, "ADC"};
    instructions[0x75] = {ADC, AddressingMode::ZERO_PAGE_X, "ADC"};
    instructions[0x6D] = {ADC, AddressingMode::ABSOLUTE, "ADC"};
    instructions[0x7D] = {ADC, AddressingMode::ABSOLUTE_X, "ADC"};
    instructions[0x79] = {ADC, AddressingMode::ABSOLUTE_Y, "ADC"};
    instructions[0x61] = {ADC, AddressingMode::INDIRECT_X, "ADC"};
    instructions[0x71] = {ADC, AddressingMode::INDIRECT_Y, "ADC"};

    instructions[0xE9] = {SBC, AddressingMode::IMMEDIATE, "SBC"};
    instructions[0xE5] = {SBC, AddressingMode::ZERO_PAGE, "SBC"};
    instructions[0xF5] = {SBC, AddressingMode::ZERO_PAGE_X, "SBC"};
    instructions[0xED] = {SBC, AddressingMode::ABSOLUTE, "SBC"};
    instructions[0xFD] = {SBC, AddressingMode::ABSOLUTE_X, "SBC"};
    instructions[0xF9] = {SBC, AddressingMode::ABSOLUTE_Y, "SBC"};
    instructions[0xE1] = {SBC, AddressingMode::INDIRECT_X, "SBC"};
    instructions[0xF1] = {SBC, AddressingMode::INDIRECT_Y, "SBC"};

    instructions[0xC6] = {DEC, AddressingMode::ZERO_PAGE, "DEC"};
    instructions[0xD6] = {DEC, AddressingMode::ZERO_PAGE_X, "DEC"};
    instructions[0xCE] = {DEC, AddressingMode::ABSOLUTE, "DEC"};
    instructions[0xDE] = {DEC, AddressingMode::ABSOLUTE_X, "DEC"};

    instructions[0x88] = {DEY, AddressingMode::IMPLIED, "DEY"};
    instructions[0xCA] = {DEX, AddressingMode::IMPLIED, "DEX"};

    instructions[0xE6] = {INC, AddressingMode::ZERO_PAGE, "INC"};
    instructions[0xF6] = {INC, AddressingMode::ZERO_PAGE_X, "INC"};
    instructions[0xEE] = {INC, AddressingMode::ABSOLUTE, "INC"};
    instructions[0xFE] = {INC, AddressingMode::ABSOLUTE_X, "INC"};

    instructions[0xC8] = {INY, AddressingMode::IMPLIED, "INY"};
    instructions[0xE8] = {INX, AddressingMode::IMPLIED, "INX"};
}


CPU::~CPU() {
}

void CPU::triggerIRQ() {
    irqPending = true;
}

void CPU::triggerNMI() {
    nmiPending = true;
}

uint16_t CPU::getAddress(AddressingMode mode) {
    switch (mode) {
        case AddressingMode::ACCUMULATOR:
            return 0;

        case AddressingMode::IMPLIED:
            return 0;

        case AddressingMode::IMMEDIATE: {
            return PC++;
        }

        case AddressingMode::ZERO_PAGE: {
            return fetch();
        }
        
        case AddressingMode::ZERO_PAGE_X: {
            return (fetch() + X) & 0xFF;
        }
        
        case AddressingMode::ZERO_PAGE_Y: {
            return (fetch() + Y) & 0xFF;
        }
        
        case AddressingMode::ABSOLUTE: {
            return fetchWord();
        }

        case AddressingMode::ABSOLUTE_X: {
            const uint16_t address = fetchWord();
            if ((address & 0xFF00) != ((address + X) & 0xFF00)) {
                stepCycles(1);
            }
            return address + X;
        }

        case AddressingMode::ABSOLUTE_Y: {
            const uint16_t address = fetchWord();
            if ((address & 0xFF00) != ((address + Y) & 0xFF00)) {
                stepCycles(1);
            }
            return address + Y;
        }

        case AddressingMode::INDIRECT: {
            const uint16_t address = fetchWord();
            stepCycles(2);
            return bus->read(address) | (bus->read((address & 0xFF00) | ((address + 1) & 0xFF)) << 8);
        }

        case AddressingMode::INDIRECT_X: {
            uint8_t address = fetch();
            stepCycles(2);
            return bus->read((address + X) & 0xFF) + (uint16_t(bus->read((address + X + 1) & 0xFF)) << 8);
        }
        
        case AddressingMode::INDIRECT_Y: {
            uint8_t base = fetch();
            uint8_t lo = bus->read(base);
            uint8_t hi = bus->read((base + 1) & 0xFF);

            uint16_t deref_base = ((uint16_t)lo) | ((uint16_t)hi << 8);
            uint16_t deref = deref_base + Y;
            if ((deref_base & 0xFF00) != (deref & 0xFF00)) {
                stepCycles(1);
            }

            stepCycles(2);
            return deref;
        }

        default:
            throw std::runtime_error("Unsupported addressing mode");
    }

    return 0;
}

void CPU::powerOn() {
    PC = bus->readWord(0xFFFC);
    SP = 0xFD;
    P = 0x24;
    A = 0x00;
    X = 0x00;
    Y = 0x00;
    cycles = 4;
}

void CPU::reset() {
    PC = bus->readWord(0xFFFC);
    SP -= 3;
    P |= INTERRUPT_DISABLE_FLAG;
    cycles = 0;
}

uint8_t CPU::fetch() {
    const uint8_t data = bus->read(PC++);
    stepCycles(1);
    return data;
}

uint16_t CPU::fetchWord() {
    uint16_t data = fetch() | (fetch() << 8);
    return data;
}

void CPU::executeOnce() {
    lastCycles = cycles;
    if (nmiPending) {  
        pushWord(PC);  
        pushByte(P & ~BREAK_FLAG);  
        P |= INTERRUPT_DISABLE_FLAG;  
        PC = bus->readWord(0xFFFA);  
        nmiPending = false;
    } 
    if (irqPending && !(P & INTERRUPT_DISABLE_FLAG)) {
        pushWord(PC);
        pushByte(P & ~BREAK_FLAG);
        P |= INTERRUPT_DISABLE_FLAG;
        PC = bus->readWord(0xFFFE);
        irqPending = false;
    } else if ((P & INTERRUPT_DISABLE_FLAG)) {
        irqPending = false;
    }    

    uint8_t opcode = fetch();
    currentOpcode = opcode;
    auto [instruction, addressingMode, name] = instructions[opcode];
    currentInstruction = name;
    instruction(this, addressingMode);
}

void CPU::pushByte(uint8_t data) {
    bus->write(0x100 | SP--, data);
    stepCycles(1);
}

void CPU::pushWord(uint16_t data) {
    pushByte(data >> 8);
    pushByte(data & 0xFF);
}

void CPU::stepCycles(size_t cycles) {        
    this->cycles += cycles;

    if (cycleCallback) {
        cycleCallback();
    }    
}


void CPU::stallCycles(size_t cycles) {
    // TODO: add this
    this->cycles += cycles;
}

uint8_t CPU::popByte() {
    stepCycles(1);
    return bus->read(0x100 | ++SP);
}

uint16_t CPU::popWord() {
    uint16_t data = popByte();
    data |= popByte() << 8;
    return data;
}
