#include <array>
#include <cpu.hpp>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>

// TODO: implement timing for page crossing on illegal opcodes

static const std::array<std::string, 256> instructionNames = {
    "BRK", "ORA", "KIL", "SLO", "DOP", "ORA", "ASL", "SLO", "PHP", "ORA", "ASL", "AAC", "TOP",
    "ORA", "ASL", "SLO", "BPL", "ORA", "KIL", "SLO", "DOP", "ORA", "ASL", "SLO", "CLC", "ORA",
    "NOP", "SLO", "TOP", "ORA", "ASL", "SLO", "JSR", "AND", "KIL", "RLA", "BIT", "AND", "ROL",
    "RLA", "PLP", "AND", "ROL", "AAC", "BIT", "AND", "ROL", "RLA", "BMI", "AND", "KIL", "RLA",
    "DOP", "AND", "ROL", "RLA", "SEC", "AND", "NOP", "RLA", "TOP", "AND", "ROL", "RLA", "RTI",
    "EOR", "KIL", "SRE", "DOP", "EOR", "LSR", "SRE", "PHA", "EOR", "LSR", "ASR", "JMP", "EOR",
    "LSR", "SRE", "BVC", "EOR", "KIL", "SRE", "DOP", "EOR", "LSR", "SRE", "CLI", "EOR", "NOP",
    "SRE", "TOP", "EOR", "LSR", "SRE", "RTS", "ADC", "KIL", "RRA", "DOP", "ADC", "ROR", "RRA",
    "PLA", "ADC", "ROR", "ARR", "JMP", "ADC", "ROR", "RRA", "BVS", "ADC", "KIL", "RRA", "DOP",
    "ADC", "ROR", "RRA", "SEI", "ADC", "NOP", "RRA", "TOP", "ADC", "ROR", "RRA", "DOP", "STA",
    "DOP", "AAX", "STY", "STA", "STX", "AAX", "DEY", "DOP", "TXA", "XAA", "STY", "STA", "STX",
    "AAX", "BCC", "STA", "KIL", "AXA", "STY", "STA", "STX", "AAX", "TYA", "STA", "TXS", "XAS",
    "SYA", "STA", "SXA", "AXA", "LDY", "LDA", "LDX", "LAX", "LDY", "LDA", "LDX", "LAX", "TAY",
    "LDA", "TAX", "ATX", "LDY", "LDA", "LDX", "LAX", "BCS", "LDA", "KIL", "LAX", "LDY", "LDA",
    "LDX", "LAX", "CLV", "LDA", "TSX", "LAR", "LDY", "LDA", "LDX", "LAX", "CPY", "CMP", "DOP",
    "DCP", "CPY", "CMP", "DEC", "DCP", "INY", "CMP", "DEX", "AXS", "CPY", "CMP", "DEC", "DCP",
    "BNE", "CMP", "KIL", "DCP", "DOP", "CMP", "DEC", "DCP", "CLD", "CMP", "NOP", "DCP", "TOP",
    "CMP", "DEC", "DCP", "CPX", "SBC", "DOP", "ISC", "CPX", "SBC", "INC", "ISC", "INX", "SBC",
    "NOP", "SBC", "CPX", "SBC", "INC", "ISC", "BEQ", "SBC", "KIL", "ISC", "DOP", "SBC", "INC",
    "ISC", "SED", "SBC", "NOP", "ISC", "TOP", "SBC", "INC", "ISC",
};

static std::string AddressingModeName(AddressingMode mode) {
    switch(mode) {
    case AddressingMode::IMMEDIATE:
        return "IMMEDIATE";
    case AddressingMode::ZERO_PAGE:
        return "ZERO_PAGE";
    case AddressingMode::ZERO_PAGE_X:
        return "ZERO_PAGE_X";
    case AddressingMode::ZERO_PAGE_Y:
        return "ZERO_PAGE_Y";
    case AddressingMode::ABSOLUTE:
        return "ABSOLUTE";
    case AddressingMode::ABSOLUTE_X:
        return "ABSOLUTE_X";
    case AddressingMode::ABSOLUTE_Y:
        return "ABSOLUTE_Y";
    case AddressingMode::INDIRECT:
        return "INDIRECT";
    case AddressingMode::INDIRECT_X:
        return "INDIRECT_X";
    case AddressingMode::INDIRECT_Y:
        return "INDIRECT_Y";
    case AddressingMode::ACCUMULATOR:
        return "ACCUMULATOR";
    case AddressingMode::RELATIVE:
        return "RELATIVE";
    case AddressingMode::IMPLIED:
        return "IMPLIED";
    }

    return "UNKNOWN";
}

static void unkownInstruction(CPU* cpu, AddressingMode mode) {
    std::cout << "opcode: " << std::hex << static_cast<int>(cpu->getCurrentOpcode()) << std::dec
              << "\n";
    throw std::runtime_error("Unknown instruction");
}

static void JMP(CPU* cpu, AddressingMode mode) {
    cpu->PC = cpu->getAddress(mode);
}

static void LDX(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->X = cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->X == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->X & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void LDA(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->A = cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void LDY(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->Y = cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->Y == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->Y & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void STX(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->bus->write(address, cpu->X);
    cpu->stepCycles(1);
}

static void STY(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->bus->write(address, cpu->Y);
    cpu->stepCycles(1);
}

static void JSR(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->pushWord(cpu->PC - 1);
    cpu->PC = address;
    cpu->stepCycles(1);
}

static void RTS(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->popWord();
    cpu->PC = address + 1;
    cpu->stepCycles(3);
}

static void NOP(CPU* cpu, AddressingMode mode) {
    cpu->stepCycles(1);
}

static void SEC(CPU* cpu, AddressingMode mode) {
    cpu->P |= CARRY_FLAG;
    cpu->stepCycles(1);
}

static void CLC(CPU* cpu, AddressingMode mode) {
    cpu->P &= ~CARRY_FLAG;
    cpu->stepCycles(1);
}

static void SEI(CPU* cpu, AddressingMode mode) {
    cpu->P |= INTERRUPT_DISABLE_FLAG;
    cpu->stepCycles(1);
}

static void CLI(CPU* cpu, AddressingMode mode) {
    cpu->P &= ~INTERRUPT_DISABLE_FLAG;
    cpu->stepCycles(1);
}

static void SED(CPU* cpu, AddressingMode mode) {
    cpu->P |= DECIMAL_MODE_FLAG;
    cpu->stepCycles(1);
}

static void CLD(CPU* cpu, AddressingMode mode) {
    cpu->P &= ~DECIMAL_MODE_FLAG;
    cpu->stepCycles(1);
}

static void CLV(CPU* cpu, AddressingMode mode) {
    cpu->P &= ~OVERFLOW_FLAG;
    cpu->stepCycles(1);
}

static void BCS(CPU* cpu, AddressingMode mode) {
    int8_t offset = cpu->bus->read(cpu->PC++);
    if(cpu->P & CARRY_FLAG) {
        cpu->stepCycles(1);
        if((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset;
    }
    cpu->stepCycles(1);
}

static void BCC(CPU* cpu, AddressingMode mode) {
    int8_t offset = cpu->bus->read(cpu->PC++);
    if(!(cpu->P & CARRY_FLAG)) {
        cpu->stepCycles(1);
        if((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset;
    }
    cpu->stepCycles(1);
}

static void BEQ(CPU* cpu, AddressingMode mode) {
    int8_t offset = cpu->bus->read(cpu->PC++);
    if(cpu->P & ZERO_FLAG) {
        cpu->stepCycles(1);
        if((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset;
    }
    cpu->stepCycles(1);
}

static void BNE(CPU* cpu, AddressingMode mode) {
    int8_t offset = cpu->bus->read(cpu->PC++);
    if(!(cpu->P & ZERO_FLAG)) {
        cpu->stepCycles(1);
        if((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset;
    }
    cpu->stepCycles(1);
}

static void BVS(CPU* cpu, AddressingMode mode) {
    int8_t offset = cpu->bus->read(cpu->PC++);
    if(cpu->P & OVERFLOW_FLAG) {
        cpu->stepCycles(1);
        if((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset;
    }
    cpu->stepCycles(1);
}

static void BVC(CPU* cpu, AddressingMode mode) {
    int8_t offset = cpu->bus->read(cpu->PC++);
    if(!(cpu->P & OVERFLOW_FLAG)) {
        cpu->stepCycles(1);
        if((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset;
    }
    cpu->stepCycles(1);
}

static void BPL(CPU* cpu, AddressingMode mode) {
    int8_t offset = cpu->bus->read(cpu->PC++);
    if(!(cpu->P & NEGATIVE_FLAG)) {
        cpu->stepCycles(1);
        if((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset;
    }
    cpu->stepCycles(1);
}

static void BMI(CPU* cpu, AddressingMode mode) {
    int8_t offset = cpu->bus->read(cpu->PC++);
    if(cpu->P & NEGATIVE_FLAG) {
        cpu->stepCycles(1);
        if((cpu->PC & 0xFF00) != ((cpu->PC + offset) & 0xFF00)) {
            cpu->stepCycles(1);
        }
        cpu->PC += offset;
    }
    cpu->stepCycles(1);
}

static void STA(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode, false);
    cpu->bus->write(address, cpu->A);
    cpu->stepCycles(1);
}

static void BIT(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~OVERFLOW_FLAG;
    if((cpu->A & data) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if(data & 0x40) {
        cpu->P |= OVERFLOW_FLAG;
    }
    cpu->stepCycles(1);
}

static void PHP(CPU* cpu, AddressingMode mode) {
    cpu->pushByte(cpu->P | BREAK_FLAG | UNUSED_FLAG);
    cpu->stepCycles(2);
}

static void PLP(CPU* cpu, AddressingMode mode) {
    cpu->P = (cpu->popByte() & 0xEF) | (cpu->P & 0x10) | 0x20;
    cpu->stepCycles(2);
}

static void PHA(CPU* cpu, AddressingMode mode) {
    cpu->pushByte(cpu->A);
    cpu->stepCycles(2);
}

static void PLA(CPU* cpu, AddressingMode mode) {
    cpu->A = cpu->popByte();
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(2);
}

// static void RTI(CPU *cpu, AddressingMode mode) {
//     cpu->P = (cpu->popByte() & 0xEF) | (cpu->P & 0x10) | 0x20;
//     cpu->PC = cpu->popWord();
//     cpu->stepCycles(1);
// }

static void RTI(CPU* cpu, AddressingMode mode) {
    cpu->P = cpu->popByte();  // Pop status register
    cpu->P |= 0x20;           // Ensure unused bit is set
    cpu->PC = cpu->popWord(); // Restore PC
}

static void AND(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->A &= cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void CMP(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint16_t result = cpu->A - data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    if((result & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if(cpu->A >= data) {
        cpu->P |= CARRY_FLAG;
    }
    cpu->stepCycles(1);
}

static void CPX(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint16_t result = cpu->X - data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    if(result == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if(cpu->X >= data) {
        cpu->P |= CARRY_FLAG;
    }
    cpu->stepCycles(1);
}

static void CPY(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint16_t result = cpu->Y - data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    if(result == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if(cpu->Y >= data) {
        cpu->P |= CARRY_FLAG;
    }
    cpu->stepCycles(1);
}

static void ORA(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->A |= cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void EOR(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->A ^= cpu->bus->read(address);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

// we will implement decimal mode later :3
static void ADC(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint16_t result = cpu->A + data + (cpu->P & CARRY_FLAG);

    if(cpu->P & DECIMAL_MODE_FLAG) {
        if((cpu->A & 0x0F) + (data & 0x0F) + (cpu->P & CARRY_FLAG) > 0x09) {
            result += 0x06;
        }
        if(result > 0x99) {
            result += 0x60;
        }
    }

    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~OVERFLOW_FLAG;
    if((result & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if(result > 0xFF) {
        cpu->P |= CARRY_FLAG;
    }
    if((cpu->A ^ result) & (data ^ result) & 0x80) {
        cpu->P |= OVERFLOW_FLAG;
    }
    cpu->A = result;
    cpu->stepCycles(1);
}

static void SBC(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint16_t result = cpu->A - data - (1 - (cpu->P & CARRY_FLAG));

    if(cpu->P & DECIMAL_MODE_FLAG) {
        if((cpu->A & 0x0F) < (data & 0x0F) + (1 - (cpu->P & CARRY_FLAG))) {
            result -= 0x06;
        }
        if(result & 0x100) {
            result -= 0x60;
        }
    }

    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~OVERFLOW_FLAG;
    if((result & 0xff) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if(result < 0x100) {
        cpu->P |= CARRY_FLAG;
    }
    if(((cpu->A ^ result) & (cpu->A ^ data) & 0x80) != 0) {
        cpu->P |= OVERFLOW_FLAG;
    }
    cpu->A = result;
    cpu->stepCycles(1);
}

static void DEC(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode, true);
    uint8_t data = cpu->bus->read(address) - 1;
    cpu->bus->write(address, data);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(2);
}

static void DEY(CPU* cpu, AddressingMode mode) {
    cpu->Y -= 1;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->Y == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->Y & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void DEX(CPU* cpu, AddressingMode mode) {
    cpu->X -= 1;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->X == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->X & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void INC(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode, true);
    uint8_t data = cpu->bus->read(address) + 1;
    cpu->bus->write(address, data);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(2);
}

static void INY(CPU* cpu, AddressingMode mode) {
    cpu->Y += 1;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->Y == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->Y & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void INX(CPU* cpu, AddressingMode mode) {
    cpu->X += 1;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->X == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->X & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void TAY(CPU* cpu, AddressingMode mode) {
    cpu->Y = cpu->A;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->Y == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->Y & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void TAX(CPU* cpu, AddressingMode mode) {
    cpu->X = cpu->A;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->X == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->X & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void TSX(CPU* cpu, AddressingMode mode) {
    cpu->X = cpu->SP;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->X == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->X & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void TYA(CPU* cpu, AddressingMode mode) {
    cpu->A = cpu->Y;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void TXA(CPU* cpu, AddressingMode mode) {
    cpu->A = cpu->X;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void TXS(CPU* cpu, AddressingMode mode) {
    cpu->SP = cpu->X;
    cpu->stepCycles(1);
}

static void LSR(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode, true);
    if(mode == AddressingMode::ACCUMULATOR) {
        cpu->P &= ~CARRY_FLAG;
        cpu->P &= ~ZERO_FLAG;
        cpu->P &= ~NEGATIVE_FLAG;
        if(cpu->A & 0x01) {
            cpu->P |= CARRY_FLAG;
        }
        cpu->A >>= 1;
        if(cpu->A == 0) {
            cpu->P |= ZERO_FLAG;
        }
        if(cpu->A & 0x80) {
            cpu->P |= NEGATIVE_FLAG;
        }
        cpu->stepCycles(2);
        return;
    }
    uint8_t data = cpu->bus->read(address);
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(data & 0x01) {
        cpu->P |= CARRY_FLAG;
    }
    data >>= 1;
    if(data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->stepCycles(2);
}

static void ASL(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode, true);
    if(mode == AddressingMode::ACCUMULATOR) {
        cpu->P &= ~CARRY_FLAG;
        cpu->P &= ~ZERO_FLAG;
        cpu->P &= ~NEGATIVE_FLAG;
        if(cpu->A & 0x80) {
            cpu->P |= CARRY_FLAG;
        }
        cpu->A <<= 1;
        if(cpu->A == 0) {
            cpu->P |= ZERO_FLAG;
        }
        if(cpu->A & 0x80) {
            cpu->P |= NEGATIVE_FLAG;
        }
        cpu->stepCycles(2);
        return;
    }
    uint8_t data = cpu->bus->read(address);
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(data & 0x80) {
        cpu->P |= CARRY_FLAG;
    }
    data <<= 1;
    if(data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->stepCycles(2);
}

static void ROR(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode, true);
    if(mode == AddressingMode::ACCUMULATOR) {
        uint8_t carry = cpu->P & CARRY_FLAG;
        cpu->P &= ~CARRY_FLAG;
        cpu->P &= ~ZERO_FLAG;
        cpu->P &= ~NEGATIVE_FLAG;
        if(cpu->A & 0x01) {
            cpu->P |= CARRY_FLAG;
        }
        cpu->A >>= 1;
        if(carry) {
            cpu->A |= 0x80;
        }
        if(cpu->A == 0) {
            cpu->P |= ZERO_FLAG;
        }
        if(cpu->A & 0x80) {
            cpu->P |= NEGATIVE_FLAG;
        }
        cpu->stepCycles(2);
        return;
    }
    uint8_t data = cpu->bus->read(address);
    uint8_t carry = cpu->P & CARRY_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(data & 0x01) {
        cpu->P |= CARRY_FLAG;
    }
    data >>= 1;
    if(carry) {
        data |= 0x80;
    }
    if(data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->stepCycles(2);
}

static void ROL(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode, true);
    if(mode == AddressingMode::ACCUMULATOR) {
        uint8_t carry = cpu->P & CARRY_FLAG;
        cpu->P &= ~CARRY_FLAG;
        cpu->P &= ~ZERO_FLAG;
        cpu->P &= ~NEGATIVE_FLAG;
        if(cpu->A & 0x80) {
            cpu->P |= CARRY_FLAG;
        }
        cpu->A <<= 1;
        if(carry) {
            cpu->A |= 0x01;
        }
        if(cpu->A == 0) {
            cpu->P |= ZERO_FLAG;
        }
        if(cpu->A & 0x80) {
            cpu->P |= NEGATIVE_FLAG;
        }
        cpu->stepCycles(2);
        return;
    }
    uint8_t data = cpu->bus->read(address);
    uint8_t carry = cpu->P & CARRY_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(data & 0x80) {
        cpu->P |= CARRY_FLAG;
    }
    data <<= 1;
    if(carry) {
        data |= 0x01;
    }
    if(data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->stepCycles(2);
}

static void BRK(CPU* cpu, AddressingMode mode) {
    cpu->PC += 1;
    cpu->pushWord(cpu->PC);
    cpu->pushByte(cpu->P | BREAK_FLAG | UNUSED_FLAG);
    cpu->P |= INTERRUPT_DISABLE_FLAG;
    cpu->PC = cpu->bus->readWord(0xFFFE);
    cpu->stepCycles(1);
}

// beware. illegal opcodes beyond!!!
static void DOP(CPU* cpu, AddressingMode mode) {
    cpu->getAddress(mode);
    cpu->stepCycles(1);
}

static void TOP(CPU* cpu, AddressingMode mode) {
    cpu->getAddress(mode);
    cpu->stepCycles(1);
}

static void LAX(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->A = data;
    cpu->X = data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void AAX(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->A & cpu->X;
    cpu->bus->write(address, data);
    cpu->stepCycles(1);
}

static void DCP(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address) - 1;
    cpu->bus->write(address, data);
    uint16_t result = cpu->A - data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    if((result & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if(cpu->A >= data) {
        cpu->P |= CARRY_FLAG;
    }
    cpu->stepCycles(3);
}

static void ISC(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address) + 1;
    cpu->bus->write(address, data);
    uint16_t result = cpu->A - data - (1 - (cpu->P & CARRY_FLAG));
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~OVERFLOW_FLAG;
    if((result & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if(result < 0x100) {
        cpu->P |= CARRY_FLAG;
    }
    if(((cpu->A ^ result) & (cpu->A ^ data) & 0x80) != 0) {
        cpu->P |= OVERFLOW_FLAG;
    }
    cpu->A = result & 0xFF;
    cpu->stepCycles(3);
}

static void SLO(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(data & 0x80) {
        cpu->P |= CARRY_FLAG;
    }
    data <<= 1;
    if((data & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->A |= data;

    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;

    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(3);
}

static void RLA(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint8_t carry = cpu->P & CARRY_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(data & 0x80) {
        cpu->P |= CARRY_FLAG;
    }
    data <<= 1;
    if(carry) {
        data |= 0x01;
    }
    if(data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->A &= data;

    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;

    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(3);
}

static void SRE(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(data & 0x01) {
        cpu->P |= CARRY_FLAG;
    }
    data >>= 1;
    if((data & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    cpu->A ^= data;

    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;

    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(3);
}

static void RRA(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    uint8_t carry = cpu->P & CARRY_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(data & 0x01) {
        cpu->P |= CARRY_FLAG;
    }
    data >>= 1;
    if(carry) {
        data |= 0x80;
    }
    if(data == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(data & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->bus->write(address, data);
    uint16_t result = cpu->A + data + (cpu->P & CARRY_FLAG);
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~OVERFLOW_FLAG;
    if((result & 0xFF) == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(result & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if(result > 0xFF) {
        cpu->P |= CARRY_FLAG;
    }
    if((cpu->A ^ result) & (data ^ result) & 0x80) {
        cpu->P |= OVERFLOW_FLAG;
    }
    cpu->A = result;
    cpu->stepCycles(3);
}

// probably not right but ok
static void XAA(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->A &= cpu->X & data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void KIL(CPU* cpu, AddressingMode mode) {
    cpu->PC -= 1;
    cpu->stepCycles(1);
}

// AND X register with accumulator and store result in stack pointer, then
// AND stack pointer with the high byte of the target address of the
// argument + 1. Store result in memory.
static void XAS(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->X = cpu->A & cpu->X;
    cpu->SP = cpu->X;
    cpu->bus->write(address, cpu->SP & ((address >> 8) + 1));
    cpu->stepCycles(2);
}

static void AAC(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->A &= data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->P &= ~CARRY_FLAG;
    if(cpu->P & NEGATIVE_FLAG) {
        cpu->P |= CARRY_FLAG;
    }
    cpu->stepCycles(1);
}

static void ASR(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->A &= data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->P &= ~CARRY_FLAG;
    if(cpu->A & 0x01) {
        cpu->P |= CARRY_FLAG;
    }
    cpu->A >>= 1;
    cpu->stepCycles(1);
}

static void ARR(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->A &= data;
    cpu->P &= ~CARRY_FLAG;
    cpu->P &= ~OVERFLOW_FLAG;
    cpu->A >>= 1;
    switch((cpu->A & 0b00110000) >> 4) {
    case 0b11:
        cpu->P |= CARRY_FLAG;
        break;
    case 0b10:
        cpu->P |= CARRY_FLAG;
        cpu->P |= OVERFLOW_FLAG;
        break;
    case 0b01:
        cpu->P |= OVERFLOW_FLAG;
        break;
    case 0b00:
        break;
    }
    cpu->stepCycles(1);
}

// i dont think this is right
static void SYA(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->bus->write(address, cpu->Y & ((address >> 8) + 1));
    cpu->stepCycles(2);
}

static void AXA(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->bus->write(address, cpu->X & cpu->A & 0x07);
    cpu->stepCycles(2);
}

static void SXA(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    cpu->bus->write(address, cpu->X & ((address >> 8) + 1));
    cpu->stepCycles(2);
}

static void ATX(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->A &= data;
    cpu->X = cpu->A;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    if(cpu->X == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->X & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    cpu->stepCycles(1);
}

static void LAR(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->A = cpu->X = cpu->SP = cpu->SP & data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;

    if(cpu->A == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->A & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }

    cpu->stepCycles(1);
}

static void AXS(CPU* cpu, AddressingMode mode) {
    uint16_t address = cpu->getAddress(mode);
    uint8_t data = cpu->bus->read(address);
    cpu->X = cpu->A & cpu->X;
    cpu->X -= data;
    cpu->P &= ~ZERO_FLAG;
    cpu->P &= ~NEGATIVE_FLAG;
    cpu->P &= ~CARRY_FLAG;
    if(cpu->X == 0) {
        cpu->P |= ZERO_FLAG;
    }
    if(cpu->X & 0x80) {
        cpu->P |= NEGATIVE_FLAG;
    }
    if(cpu->X >= data) {
        cpu->P |= CARRY_FLAG;
    }
    cpu->stepCycles(1);
}

CPU::CPU(Bus* bus) {
    this->bus = bus;

    instructions.fill({unkownInstruction, AddressingMode::IMPLIED});

    instructions[0x00] = {BRK, AddressingMode::IMPLIED};
    instructions[0x01] = {ORA, AddressingMode::INDIRECT_X};
    instructions[0x02] = {KIL, AddressingMode::IMPLIED};
    instructions[0x03] = {SLO, AddressingMode::INDIRECT_X};
    instructions[0x04] = {DOP, AddressingMode::ZERO_PAGE};
    instructions[0x05] = {ORA, AddressingMode::ZERO_PAGE};
    instructions[0x06] = {ASL, AddressingMode::ZERO_PAGE};
    instructions[0x07] = {SLO, AddressingMode::ZERO_PAGE};
    instructions[0x08] = {PHP, AddressingMode::IMPLIED};
    instructions[0x09] = {ORA, AddressingMode::IMMEDIATE};
    instructions[0x0A] = {ASL, AddressingMode::ACCUMULATOR};
    instructions[0x0B] = {AAC, AddressingMode::IMMEDIATE};
    instructions[0x0C] = {TOP, AddressingMode::ABSOLUTE};
    instructions[0x0D] = {ORA, AddressingMode::ABSOLUTE};
    instructions[0x0E] = {ASL, AddressingMode::ABSOLUTE};
    instructions[0x0F] = {SLO, AddressingMode::ABSOLUTE};
    instructions[0x10] = {BPL, AddressingMode::RELATIVE};
    instructions[0x11] = {ORA, AddressingMode::INDIRECT_Y};
    instructions[0x12] = {KIL, AddressingMode::IMPLIED};
    instructions[0x13] = {SLO, AddressingMode::INDIRECT_Y};
    instructions[0x14] = {DOP, AddressingMode::ZERO_PAGE_X};
    instructions[0x15] = {ORA, AddressingMode::ZERO_PAGE_X};
    instructions[0x16] = {ASL, AddressingMode::ZERO_PAGE_X};
    instructions[0x17] = {SLO, AddressingMode::ZERO_PAGE_X};
    instructions[0x18] = {CLC, AddressingMode::IMPLIED};
    instructions[0x19] = {ORA, AddressingMode::ABSOLUTE_Y};
    instructions[0x1A] = {NOP, AddressingMode::IMPLIED};
    instructions[0x1B] = {SLO, AddressingMode::ABSOLUTE_Y};
    instructions[0x1C] = {TOP, AddressingMode::ABSOLUTE_X};
    instructions[0x1D] = {ORA, AddressingMode::ABSOLUTE_X};
    instructions[0x1E] = {ASL, AddressingMode::ABSOLUTE_X};
    instructions[0x1F] = {SLO, AddressingMode::ABSOLUTE_X};
    instructions[0x20] = {JSR, AddressingMode::ABSOLUTE};
    instructions[0x21] = {AND, AddressingMode::INDIRECT_X};
    instructions[0x22] = {KIL, AddressingMode::IMPLIED};
    instructions[0x23] = {RLA, AddressingMode::INDIRECT_X};
    instructions[0x24] = {BIT, AddressingMode::ZERO_PAGE};
    instructions[0x25] = {AND, AddressingMode::ZERO_PAGE};
    instructions[0x26] = {ROL, AddressingMode::ZERO_PAGE};
    instructions[0x27] = {RLA, AddressingMode::ZERO_PAGE};
    instructions[0x28] = {PLP, AddressingMode::IMPLIED};
    instructions[0x29] = {AND, AddressingMode::IMMEDIATE};
    instructions[0x2A] = {ROL, AddressingMode::ACCUMULATOR};
    instructions[0x2B] = {AAC, AddressingMode::IMMEDIATE};
    instructions[0x2C] = {BIT, AddressingMode::ABSOLUTE};
    instructions[0x2D] = {AND, AddressingMode::ABSOLUTE};
    instructions[0x2E] = {ROL, AddressingMode::ABSOLUTE};
    instructions[0x2F] = {RLA, AddressingMode::ABSOLUTE};
    instructions[0x30] = {BMI, AddressingMode::RELATIVE};
    instructions[0x31] = {AND, AddressingMode::INDIRECT_Y};
    instructions[0x32] = {KIL, AddressingMode::IMPLIED};
    instructions[0x33] = {RLA, AddressingMode::INDIRECT_Y};
    instructions[0x34] = {DOP, AddressingMode::ZERO_PAGE_X};
    instructions[0x35] = {AND, AddressingMode::ZERO_PAGE_X};
    instructions[0x36] = {ROL, AddressingMode::ZERO_PAGE_X};
    instructions[0x37] = {RLA, AddressingMode::ZERO_PAGE_X};
    instructions[0x38] = {SEC, AddressingMode::IMPLIED};
    instructions[0x39] = {AND, AddressingMode::ABSOLUTE_Y};
    instructions[0x3A] = {NOP, AddressingMode::IMPLIED};
    instructions[0x3B] = {RLA, AddressingMode::ABSOLUTE_Y};
    instructions[0x3C] = {TOP, AddressingMode::ABSOLUTE_X};
    instructions[0x3D] = {AND, AddressingMode::ABSOLUTE_X};
    instructions[0x3E] = {ROL, AddressingMode::ABSOLUTE_X};
    instructions[0x3F] = {RLA, AddressingMode::ABSOLUTE_X};
    instructions[0x40] = {RTI, AddressingMode::IMPLIED};
    instructions[0x41] = {EOR, AddressingMode::INDIRECT_X};
    instructions[0x42] = {KIL, AddressingMode::IMPLIED};
    instructions[0x43] = {SRE, AddressingMode::INDIRECT_X};
    instructions[0x44] = {DOP, AddressingMode::ZERO_PAGE};
    instructions[0x45] = {EOR, AddressingMode::ZERO_PAGE};
    instructions[0x46] = {LSR, AddressingMode::ZERO_PAGE};
    instructions[0x47] = {SRE, AddressingMode::ZERO_PAGE};
    instructions[0x48] = {PHA, AddressingMode::IMPLIED};
    instructions[0x49] = {EOR, AddressingMode::IMMEDIATE};
    instructions[0x4A] = {LSR, AddressingMode::ACCUMULATOR};
    instructions[0x4B] = {ASR, AddressingMode::IMMEDIATE};
    instructions[0x4C] = {JMP, AddressingMode::ABSOLUTE};
    instructions[0x4D] = {EOR, AddressingMode::ABSOLUTE};
    instructions[0x4E] = {LSR, AddressingMode::ABSOLUTE};
    instructions[0x4F] = {SRE, AddressingMode::ABSOLUTE};
    instructions[0x50] = {BVC, AddressingMode::RELATIVE};
    instructions[0x51] = {EOR, AddressingMode::INDIRECT_Y};
    instructions[0x52] = {KIL, AddressingMode::IMPLIED};
    instructions[0x53] = {SRE, AddressingMode::INDIRECT_Y};
    instructions[0x54] = {DOP, AddressingMode::ZERO_PAGE_X};
    instructions[0x55] = {EOR, AddressingMode::ZERO_PAGE_X};
    instructions[0x56] = {LSR, AddressingMode::ZERO_PAGE_X};
    instructions[0x57] = {SRE, AddressingMode::ZERO_PAGE_X};
    instructions[0x58] = {CLI, AddressingMode::IMPLIED};
    instructions[0x59] = {EOR, AddressingMode::ABSOLUTE_Y};
    instructions[0x5A] = {NOP, AddressingMode::IMPLIED};
    instructions[0x5B] = {SRE, AddressingMode::ABSOLUTE_Y};
    instructions[0x5C] = {TOP, AddressingMode::ABSOLUTE_X};
    instructions[0x5D] = {EOR, AddressingMode::ABSOLUTE_X};
    instructions[0x5E] = {LSR, AddressingMode::ABSOLUTE_X};
    instructions[0x5F] = {SRE, AddressingMode::ABSOLUTE_X};
    instructions[0x60] = {RTS, AddressingMode::IMPLIED};
    instructions[0x61] = {ADC, AddressingMode::INDIRECT_X};
    instructions[0x62] = {KIL, AddressingMode::IMPLIED};
    instructions[0x63] = {RRA, AddressingMode::INDIRECT_X};
    instructions[0x64] = {DOP, AddressingMode::ZERO_PAGE};
    instructions[0x65] = {ADC, AddressingMode::ZERO_PAGE};
    instructions[0x66] = {ROR, AddressingMode::ZERO_PAGE};
    instructions[0x67] = {RRA, AddressingMode::ZERO_PAGE};
    instructions[0x68] = {PLA, AddressingMode::IMPLIED};
    instructions[0x69] = {ADC, AddressingMode::IMMEDIATE};
    instructions[0x6A] = {ROR, AddressingMode::ACCUMULATOR};
    instructions[0x6B] = {ARR, AddressingMode::IMMEDIATE};
    instructions[0x6C] = {JMP, AddressingMode::INDIRECT};
    instructions[0x6D] = {ADC, AddressingMode::ABSOLUTE};
    instructions[0x6E] = {ROR, AddressingMode::ABSOLUTE};
    instructions[0x6F] = {RRA, AddressingMode::ABSOLUTE};
    instructions[0x70] = {BVS, AddressingMode::RELATIVE};
    instructions[0x71] = {ADC, AddressingMode::INDIRECT_Y};
    instructions[0x72] = {KIL, AddressingMode::IMPLIED};
    instructions[0x73] = {RRA, AddressingMode::INDIRECT_Y};
    instructions[0x74] = {DOP, AddressingMode::ZERO_PAGE_X};
    instructions[0x75] = {ADC, AddressingMode::ZERO_PAGE_X};
    instructions[0x76] = {ROR, AddressingMode::ZERO_PAGE_X};
    instructions[0x77] = {RRA, AddressingMode::ZERO_PAGE_X};
    instructions[0x78] = {SEI, AddressingMode::IMPLIED};
    instructions[0x79] = {ADC, AddressingMode::ABSOLUTE_Y};
    instructions[0x7A] = {NOP, AddressingMode::IMPLIED};
    instructions[0x7B] = {RRA, AddressingMode::ABSOLUTE_Y};
    instructions[0x7C] = {TOP, AddressingMode::ABSOLUTE_X};
    instructions[0x7D] = {ADC, AddressingMode::ABSOLUTE_X};
    instructions[0x7E] = {ROR, AddressingMode::ABSOLUTE_X};
    instructions[0x7F] = {RRA, AddressingMode::ABSOLUTE_X};
    instructions[0x80] = {DOP, AddressingMode::IMMEDIATE};
    instructions[0x81] = {STA, AddressingMode::INDIRECT_X};
    instructions[0x82] = {DOP, AddressingMode::IMMEDIATE};
    instructions[0x83] = {AAX, AddressingMode::INDIRECT_X};
    instructions[0x84] = {STY, AddressingMode::ZERO_PAGE};
    instructions[0x85] = {STA, AddressingMode::ZERO_PAGE};
    instructions[0x86] = {STX, AddressingMode::ZERO_PAGE};
    instructions[0x87] = {AAX, AddressingMode::ZERO_PAGE};
    instructions[0x88] = {DEY, AddressingMode::IMPLIED};
    instructions[0x89] = {DOP, AddressingMode::IMMEDIATE};
    instructions[0x8A] = {TXA, AddressingMode::IMPLIED};
    instructions[0x8B] = {XAA, AddressingMode::IMMEDIATE};
    instructions[0x8C] = {STY, AddressingMode::ABSOLUTE};
    instructions[0x8D] = {STA, AddressingMode::ABSOLUTE};
    instructions[0x8E] = {STX, AddressingMode::ABSOLUTE};
    instructions[0x8F] = {AAX, AddressingMode::ABSOLUTE};
    instructions[0x90] = {BCC, AddressingMode::RELATIVE};
    instructions[0x91] = {STA, AddressingMode::INDIRECT_Y};
    instructions[0x92] = {KIL, AddressingMode::IMPLIED};
    instructions[0x93] = {AXA, AddressingMode::INDIRECT_Y};
    instructions[0x94] = {STY, AddressingMode::ZERO_PAGE_X};
    instructions[0x95] = {STA, AddressingMode::ZERO_PAGE_X};
    instructions[0x96] = {STX, AddressingMode::ZERO_PAGE_Y};
    instructions[0x97] = {AAX, AddressingMode::ZERO_PAGE_Y};
    instructions[0x98] = {TYA, AddressingMode::IMPLIED};
    instructions[0x99] = {STA, AddressingMode::ABSOLUTE_Y};
    instructions[0x9A] = {TXS, AddressingMode::IMPLIED};
    instructions[0x9B] = {XAS, AddressingMode::ABSOLUTE_Y};
    instructions[0x9C] = {SYA, AddressingMode::ABSOLUTE_X};
    instructions[0x9D] = {STA, AddressingMode::ABSOLUTE_X};
    instructions[0x9E] = {SXA, AddressingMode::ABSOLUTE_Y};
    instructions[0x9F] = {AXA, AddressingMode::ABSOLUTE_Y};
    instructions[0xA0] = {LDY, AddressingMode::IMMEDIATE};
    instructions[0xA1] = {LDA, AddressingMode::INDIRECT_X};
    instructions[0xA2] = {LDX, AddressingMode::IMMEDIATE};
    instructions[0xA3] = {LAX, AddressingMode::INDIRECT_X};
    instructions[0xA4] = {LDY, AddressingMode::ZERO_PAGE};
    instructions[0xA5] = {LDA, AddressingMode::ZERO_PAGE};
    instructions[0xA6] = {LDX, AddressingMode::ZERO_PAGE};
    instructions[0xA7] = {LAX, AddressingMode::ZERO_PAGE};
    instructions[0xA8] = {TAY, AddressingMode::IMPLIED};
    instructions[0xA9] = {LDA, AddressingMode::IMMEDIATE};
    instructions[0xAA] = {TAX, AddressingMode::IMPLIED};
    instructions[0xAB] = {ATX, AddressingMode::IMMEDIATE};
    instructions[0xAC] = {LDY, AddressingMode::ABSOLUTE};
    instructions[0xAD] = {LDA, AddressingMode::ABSOLUTE};
    instructions[0xAE] = {LDX, AddressingMode::ABSOLUTE};
    instructions[0xAF] = {LAX, AddressingMode::ABSOLUTE};
    instructions[0xB0] = {BCS, AddressingMode::RELATIVE};
    instructions[0xB1] = {LDA, AddressingMode::INDIRECT_Y};
    instructions[0xB2] = {KIL, AddressingMode::IMPLIED};
    instructions[0xB3] = {LAX, AddressingMode::INDIRECT_Y};
    instructions[0xB4] = {LDY, AddressingMode::ZERO_PAGE_X};
    instructions[0xB5] = {LDA, AddressingMode::ZERO_PAGE_X};
    instructions[0xB6] = {LDX, AddressingMode::ZERO_PAGE_Y};
    instructions[0xB7] = {LAX, AddressingMode::ZERO_PAGE_Y};
    instructions[0xB8] = {CLV, AddressingMode::IMPLIED};
    instructions[0xB9] = {LDA, AddressingMode::ABSOLUTE_Y};
    instructions[0xBA] = {TSX, AddressingMode::IMPLIED};
    instructions[0xBB] = {LAR, AddressingMode::ABSOLUTE_Y};
    instructions[0xBC] = {LDY, AddressingMode::ABSOLUTE_X};
    instructions[0xBD] = {LDA, AddressingMode::ABSOLUTE_X};
    instructions[0xBE] = {LDX, AddressingMode::ABSOLUTE_Y};
    instructions[0xBF] = {LAX, AddressingMode::ABSOLUTE_Y};
    instructions[0xC0] = {CPY, AddressingMode::IMMEDIATE};
    instructions[0xC1] = {CMP, AddressingMode::INDIRECT_X};
    instructions[0xC2] = {DOP, AddressingMode::IMMEDIATE};
    instructions[0xC3] = {DCP, AddressingMode::INDIRECT_X};
    instructions[0xC4] = {CPY, AddressingMode::ZERO_PAGE};
    instructions[0xC5] = {CMP, AddressingMode::ZERO_PAGE};
    instructions[0xC6] = {DEC, AddressingMode::ZERO_PAGE};
    instructions[0xC7] = {DCP, AddressingMode::ZERO_PAGE};
    instructions[0xC8] = {INY, AddressingMode::IMPLIED};
    instructions[0xC9] = {CMP, AddressingMode::IMMEDIATE};
    instructions[0xCA] = {DEX, AddressingMode::IMPLIED};
    instructions[0xCB] = {AXS, AddressingMode::IMMEDIATE};
    instructions[0xCC] = {CPY, AddressingMode::ABSOLUTE};
    instructions[0xCD] = {CMP, AddressingMode::ABSOLUTE};
    instructions[0xCE] = {DEC, AddressingMode::ABSOLUTE};
    instructions[0xCF] = {DCP, AddressingMode::ABSOLUTE};
    instructions[0xD0] = {BNE, AddressingMode::RELATIVE};
    instructions[0xD1] = {CMP, AddressingMode::INDIRECT_Y};
    instructions[0xD2] = {KIL, AddressingMode::IMPLIED};
    instructions[0xD3] = {DCP, AddressingMode::INDIRECT_Y};
    instructions[0xD4] = {DOP, AddressingMode::ZERO_PAGE_X};
    instructions[0xD5] = {CMP, AddressingMode::ZERO_PAGE_X};
    instructions[0xD6] = {DEC, AddressingMode::ZERO_PAGE_X};
    instructions[0xD7] = {DCP, AddressingMode::ZERO_PAGE_X};
    instructions[0xD8] = {CLD, AddressingMode::IMPLIED};
    instructions[0xD9] = {CMP, AddressingMode::ABSOLUTE_Y};
    instructions[0xDA] = {NOP, AddressingMode::IMPLIED};
    instructions[0xDB] = {DCP, AddressingMode::ABSOLUTE_Y};
    instructions[0xDC] = {TOP, AddressingMode::ABSOLUTE_X};
    instructions[0xDD] = {CMP, AddressingMode::ABSOLUTE_X};
    instructions[0xDE] = {DEC, AddressingMode::ABSOLUTE_X};
    instructions[0xDF] = {DCP, AddressingMode::ABSOLUTE_X};
    instructions[0xE0] = {CPX, AddressingMode::IMMEDIATE};
    instructions[0xE1] = {SBC, AddressingMode::INDIRECT_X};
    instructions[0xE2] = {DOP, AddressingMode::IMMEDIATE};
    instructions[0xE3] = {ISC, AddressingMode::INDIRECT_X};
    instructions[0xE4] = {CPX, AddressingMode::ZERO_PAGE};
    instructions[0xE5] = {SBC, AddressingMode::ZERO_PAGE};
    instructions[0xE6] = {INC, AddressingMode::ZERO_PAGE};
    instructions[0xE7] = {ISC, AddressingMode::ZERO_PAGE};
    instructions[0xE8] = {INX, AddressingMode::IMPLIED};
    instructions[0xE9] = {SBC, AddressingMode::IMMEDIATE};
    instructions[0xEA] = {NOP, AddressingMode::IMPLIED};
    instructions[0xEB] = {SBC, AddressingMode::IMMEDIATE};
    instructions[0xEC] = {CPX, AddressingMode::ABSOLUTE};
    instructions[0xED] = {SBC, AddressingMode::ABSOLUTE};
    instructions[0xEE] = {INC, AddressingMode::ABSOLUTE};
    instructions[0xEF] = {ISC, AddressingMode::ABSOLUTE};
    instructions[0xF0] = {BEQ, AddressingMode::RELATIVE};
    instructions[0xF1] = {SBC, AddressingMode::INDIRECT_Y};
    instructions[0xF2] = {KIL, AddressingMode::IMPLIED};
    instructions[0xF3] = {ISC, AddressingMode::INDIRECT_Y};
    instructions[0xF4] = {DOP, AddressingMode::ZERO_PAGE_X};
    instructions[0xF5] = {SBC, AddressingMode::ZERO_PAGE_X};
    instructions[0xF6] = {INC, AddressingMode::ZERO_PAGE_X};
    instructions[0xF7] = {ISC, AddressingMode::ZERO_PAGE_X};
    instructions[0xF8] = {SED, AddressingMode::IMPLIED};
    instructions[0xF9] = {SBC, AddressingMode::ABSOLUTE_Y};
    instructions[0xFA] = {NOP, AddressingMode::IMPLIED};
    instructions[0xFB] = {ISC, AddressingMode::ABSOLUTE_Y};
    instructions[0xFC] = {TOP, AddressingMode::ABSOLUTE_X};
    instructions[0xFD] = {SBC, AddressingMode::ABSOLUTE_X};
    instructions[0xFE] = {INC, AddressingMode::ABSOLUTE_X};
    instructions[0xFF] = {ISC, AddressingMode::ABSOLUTE_X};
}

CPU::~CPU() {
}

void CPU::triggerIRQ() {
    irqPending = true;
}

void CPU::triggerNMI() {
    nmiPending = true;
}

uint16_t CPU::getAddress(AddressingMode mode, bool alwaysCrossPage) {
    switch(mode) {
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
        stepCycles(1);
        return (fetch() + X) & 0xFF;
    }

    case AddressingMode::ZERO_PAGE_Y: {
        stepCycles(1);
        return (fetch() + Y) & 0xFF;
    }

    case AddressingMode::ABSOLUTE: {
        return fetchWord();
    }

    case AddressingMode::ABSOLUTE_X: {
        const uint16_t address = fetchWord();
        if((address & 0xFF00) != ((address + X) & 0xFF00) || alwaysCrossPage) {
            stepCycles(1);
        }
        return address + X;
    }

    case AddressingMode::ABSOLUTE_Y: {
        const uint16_t address = fetchWord();
        if((address & 0xFF00) != ((address + Y) & 0xFF00) || alwaysCrossPage) {
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
        stepCycles(2); // might need to change this to 3
        return bus->read((address + X) & 0xFF) +
               (uint16_t(bus->read((address + X + 1) & 0xFF)) << 8);
    }

    case AddressingMode::INDIRECT_Y: {
        uint8_t base = fetch();
        uint8_t lo = bus->read(base);
        uint8_t hi = bus->read((base + 1) & 0xFF);

        uint16_t deref_base = ((uint16_t)lo) | ((uint16_t)hi << 8);
        uint16_t deref = deref_base + Y;
        if((deref_base & 0xFF00) != (deref & 0xFF00) || alwaysCrossPage) {
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

// notes
// TODO: fix page crossing executing everywhere

void CPU::executeOnce() {
    lastCycles = cycles;
    if(nmiPending) {
        pushWord(PC);
        pushByte(P & ~BREAK_FLAG);
        P |= INTERRUPT_DISABLE_FLAG;
        PC = bus->readWord(0xFFFA);
        nmiPending = false;
    }
    if(irqPending && !(P & INTERRUPT_DISABLE_FLAG)) {
        pushWord(PC);
        pushByte(P & ~BREAK_FLAG);
        P |= INTERRUPT_DISABLE_FLAG;
        PC = bus->readWord(0xFFFE);
        irqPending = false;
    } else if((P & INTERRUPT_DISABLE_FLAG)) {
        irqPending = false;
    }

    uint8_t opcode = fetch();
    currentOpcode = opcode;
    auto [instruction, addressingMode] = instructions[opcode];
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
    for(size_t i = 0; i < cycles; i++) {
#ifndef NO_MMIO
        bus->cia1->tick();
        bus->cia2->tick();
        bus->vic->tick();
#endif
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
