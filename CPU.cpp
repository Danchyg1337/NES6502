#pragma once

#include "CPU.h"
#include "NES6502.h"
#include "Defines.h"

bool CPU::Load(uint8_t* program, size_t size) {
    memory = new uint8_t[0x10000];
    PRGROM.resize(size);
    memcpy(PRGROM.data(), program, size);
    Reset();
    if (LOGGER) {
        fopen_s(&log, "CPUlog.txt", "w");
    }
    return true;
}

void CPU::ConnectToNes(NES* nes) {
    this->nes = nes;
}

void CPU::NMI() {
    StackPush16b(PC);
    StackPush8b(SR);
    SetFlag(FLAGS::B, false);
    SetFlag(FLAGS::I, true);
    PC = (uint16_t(Read(0xFFFA + 1)) << 8) | uint16_t(Read(0xFFFA));
    clockCycle += 8;
}

void CPU::Reset() {
    for (int t = 0; t <= 0xFFFF; t++) memory[t] = 0;
    SP = 0xFD;
    PC = (uint16_t(Read(0xFFFC + 1)) << 8) | uint16_t(Read(0xFFFC));
    //PC = 0xC000;
    SR = FLAGS::IGNORED;
    SR |= FLAGS::I;
    A = 0;
    X = 0;
    Y = 0;
}

void CPU::IRQ() {
    if (SR & FLAGS::I) return;
    StackPush16b(PC);
    StackPush8b(SR);
    SetFlag(FLAGS::B, false);
    SetFlag(FLAGS::I, true);
    PC = (uint16_t(Read(0xFFFE + 1)) << 8) | uint16_t(Read(0xFFFE));
    clockCycle += 7;
}

void CPU::StackPush16b(uint16_t value) {
    StackPush8b(value >> 8);
    StackPush8b(value);
}

void CPU::StackPush8b(uint8_t value) {
    Write(stackBottom + SP, value);
    SP--;
}

uint16_t CPU::StackPop16b() {
    uint8_t MSB = StackPop8b();
    return (uint16_t(StackPop8b()) << 8) | uint16_t(MSB);
}
uint8_t CPU::StackPop8b() {
    SP++;
    uint8_t res = Read(stackBottom + SP);
    Write(stackBottom + SP, 0);
    return res;
}

uint8_t CPU::Fetch() {
    uint8_t fetched = Read(PC);
    PC++;
    return fetched;
}

void CPU::Run() {
    while (!(SR & FLAGS::B)) {
        Call();
    }
}

void CPU::Step() {
    Call();
}

void CPU::Call() {
    if (clockCycle > 0) {
        clockCycle--;
        return;
    }
    uint8_t opcode = Fetch();
    if (LOGGER) {
        if (log) fprintf_s(log, "%04X %s        A:%02X X:%02X Y:%02X SR:%02X SP:%02X PPU: %03i, %03i CYC:%i\n", PC - 1, instructions[opcode].name.data(), A, X, Y, SR, SP, nes->PPU2C02.horiLines, nes->PPU2C02.clockCycle, nes->clockCycle);
    }
    if (instructions.find(opcode) != instructions.end()) {
        (this->*instructions[opcode].mode)(instructions[opcode].command);
        clockCycle += instructions[opcode].cycles;
    }
    else
        InvalidCommand(opcode);
    SR |= FLAGS::IGNORED;
}

void CPU::InvalidCommand(uint8_t code) {
    printf("Invalid command : %02X\n", code);
    system("pause");
}

void CPU::SetFlag(FLAGS flag, bool set) {
    if (set)
        SR |= flag;
    else
        SR &= ~flag;
}

uint8_t& CPU::Read(uint16_t addr) {
    //RAM
    if (addr >= 0x0000 && addr < 0x0800) {
        return memory[addr];
    }
    //Mirrors of RAM
    else if (addr >= 0x0800 && addr < 0x2000) {
        return Read(addr % 0x0800);
    }
    //PPU Registers
    else if (addr >= 0x2000 && addr < 0x2008) {
        if (!nes) return memory[addr];
        return nes->ReadPPU(addr);
    }
    //Mirrors of PPU Registers
    else if (addr >= 0x2008 && addr < 0x4000) {
        return Read((addr - 0x2000) % 0x0008);
    }
    //APU Registers
    else if (addr >= 0x4000 && addr < 0x4020) {
        if (addr == 0x4016) {
            controllerStatus = controllerValue & FLAGS::Z;
            if (!controllerStrobe) {
                controllerStatus = (controllerValue >> controllerIndex) & 0x1;
                controllerIndex--;
                if (controllerIndex < 0) controllerIndex++;
            }
            return controllerStatus;
        }
        return memory[addr];
    }
    //Cartridge Expansion ROM
    else if (addr >= 0x4020 && addr < 0x6000) {

    }
    //SRAM
    else if (addr >= 0x6000 && addr < 0x8000) {
        return memory[addr];
    }
    //PRG-ROM
    else if (addr >= 0x8000 && addr <= 0xFFFF) {
        if (PRGROM.size() > 0x4000)
            return PRGROM[addr - 0x8000];
        return PRGROM[(addr - 0x8000) % 0x4000];
    }
}

void CPU::Write(uint16_t addr, uint8_t value) {
    //RAM
    if (addr >= 0x0000 && addr < 0x0800) {
        memory[addr] = value;
    }
    //Mirrors of RAM
    else if (addr >= 0x0800 && addr < 0x2000) {
        Write(addr % 0x0800, value);
    }
    //PPU Registers
    else if (addr >= 0x2000 && addr < 0x2008) {
        if (!nes) memory[addr] = value;
        else nes->WritePPU(addr, value);
    }
    //Mirrors of PPU Registers
    else if (addr >= 0x2008 && addr < 0x4000) {
        Write((addr - 0x2000) % 0x0008, value);
    }
    //APU Registers
    else if (addr >= 0x4000 && addr < 0x4020) {
        if (addr == 0x4016) {
            controllerStrobe = value & 0x1;
            if (controllerStrobe) controllerIndex = 7;
        }
        else
            memory[addr] = value;
    }
    //Cartridge Expansion ROM
    else if (addr >= 0x4020 && addr < 0x6000) {
        //ROM is Read-Only
    }
    //SRAM
    else if (addr >= 0x6000 && addr < 0x8000) {
        memory[addr] = value;
    }
    //PRG-ROM
    else if (addr >= 0x8000 && addr < 0xFFFF) {
        //ROM is Read-Only
    }
}

//addressing modes 
void CPU::IMP(void (CPU::* command)()) {
    (this->*command)();
}

void CPU::ACC(void (CPU::* command)()) {
    isValueRegister = false;
    (this->*command)();
}

void CPU::IMM(void (CPU::* command)()) {
    value = Fetch();
    isValueRegister = false;
    (this->*command)();
}

void CPU::ZPG(void (CPU::* command)()) {
    value = Fetch();
    isValueRegister = true;
    (this->*command)();
}

void CPU::ZPGX(void (CPU::* command)()) {
    value = uint8_t(Fetch() + int8_t(X));
    isValueRegister = true;
    (this->*command)();
}

void CPU::ZPGY(void (CPU::* command)()) {
    value = uint8_t(Fetch() + int8_t(Y));
    isValueRegister = true;
    (this->*command)();
}

void CPU::RLT(void (CPU::* command)()) {
    value = Fetch();
    isValueRegister = false;
    (this->*command)();
}

void CPU::ABS(void (CPU::* command)()) {
    uint16_t LSB = Fetch();
    uint16_t MSB = Fetch();
    value = (MSB << 8) | LSB;
    isValueRegister = true;
    (this->*command)();
}

void CPU::ABSX(void (CPU::* command)()) {
    uint16_t LSB = Fetch();
    uint16_t MSB = Fetch();
    value = ((MSB << 8) | LSB) + X;
    isValueRegister = true;
    if ((value & 0xFF00) != (MSB << 8)) clockCycle++;
    (this->*command)();
}

void CPU::ABSY(void (CPU::* command)()) {
    uint16_t LSB = Fetch();
    uint16_t MSB = Fetch();
    value = ((MSB << 8) | LSB) + Y;
    isValueRegister = true;
    if ((value & 0xFF00) != (MSB << 8)) clockCycle++;
    (this->*command)();
}

void CPU::IND(void (CPU::* command)()) {
    uint16_t LSB = Fetch();
    uint16_t MSB = Fetch();
    uint16_t pos = ((MSB << 8) | LSB);
    if (LSB == 0x00FF)
        value = (Read(pos & 0xFF00) << 8) | Read(pos + 1);
    else
        value = (Read(pos + 1) << 8) | Read(pos);
    isValueRegister = true;
    (this->*command)();
}

void CPU::INDX(void (CPU::* command)()) {
    uint8_t Xval = Fetch();
    uint8_t pos = Xval + int8_t(X);
    value = Read(pos);
    value |= uint16_t(Read(pos + 1)) << 8;
    isValueRegister = true;
    (this->*command)();
}

void CPU::INDY(void (CPU::* command)()) {
    uint8_t Yval = Fetch();
    value = Read(Yval);
    uint16_t MSB = uint16_t(Read(Yval + 1)) << 8;
    value |= MSB;
    value += Y;
    isValueRegister = true;
    if ((value & 0xFF00) != (MSB << 8)) clockCycle++;
    (this->*command)();
}

//instructions
void CPU::LDA() {
    if (isValueRegister) A = Read(value);
    else A = value;
    SetFlag(FLAGS::Z, A == 0);
    SetFlag(FLAGS::N, FLAGS::N & A);
}

void CPU::LDX() {
    if (isValueRegister) X = Read(value);
    else X = value;
    SetFlag(FLAGS::Z, X == 0);
    SetFlag(FLAGS::N, FLAGS::N & X);
}

void CPU::LDY() {
    if (isValueRegister) Y = Read(value);
    else Y = value;
    SetFlag(FLAGS::Z, Y == 0);
    SetFlag(FLAGS::N, FLAGS::N & Y);
}

void CPU::TAX() {
    X = A;
    SetFlag(FLAGS::Z, X == 0);
    SetFlag(FLAGS::N, FLAGS::N & X);
}

void CPU::TSX() {
    X = SP;
    SetFlag(FLAGS::Z, X == 0);
    SetFlag(FLAGS::N, FLAGS::N & X);
}

void CPU::TAY() {
    Y = A;
    SetFlag(FLAGS::Z, Y == 0);
    SetFlag(FLAGS::N, FLAGS::N & Y);
}

void CPU::TYA() {
    A = Y;
    SetFlag(FLAGS::Z, A == 0);
    SetFlag(FLAGS::N, FLAGS::N & A);
}

void CPU::INX() {
    X++;
    SetFlag(FLAGS::Z, X == 0);
    SetFlag(FLAGS::N, FLAGS::N & X);
}

void CPU::INY() {
    Y++;
    SetFlag(FLAGS::Z, Y == 0);
    SetFlag(FLAGS::N, FLAGS::N & Y);
}

void CPU::ADC() {                                        //not compeled
    if (isValueRegister) value = Read(value);
    uint8_t Acopy = A;
    A = A + value + (SR & FLAGS::C);
    SetFlag(FLAGS::C, A < Acopy);
    SetFlag(FLAGS::Z, A == 0);
    SetFlag(FLAGS::V, (~(Acopy ^ value) & (A ^ Acopy)) & FLAGS::N);
    SetFlag(FLAGS::N, FLAGS::N & A);
}

void CPU::SBC() {                                        //not compeled
    if (isValueRegister) value = Read(value);
    uint8_t Acopy = A;
    value ^= 0x00FF;
    A = A + value + (SR & FLAGS::C);
    SetFlag(FLAGS::C, A > Acopy);
    SetFlag(FLAGS::Z, A == 0);
    SetFlag(FLAGS::V, (A ^ Acopy) & (A ^ value) & FLAGS::N);
    SetFlag(FLAGS::N, FLAGS::N & A);
}

void CPU::AND() {
    if (isValueRegister) A &= Read(value);
    else A &= value;
    SetFlag(FLAGS::Z, A == 0);
    SetFlag(FLAGS::N, FLAGS::N & A);
}

void CPU::STA() {
    Write(value, A);
}
void CPU::STX() {
    Write(value, X);
}
void CPU::STY() {
    Write(value, Y);
}

void CPU::JSR() {
    StackPush16b(PC);
    PC = value;
}

void CPU::RTS() {
    PC = StackPop16b();
}

void CPU::CLC() {
    SetFlag(FLAGS::C, false);
}

void CPU::CMP() {
    if (isValueRegister) 
        value = Read(value);
    SetFlag(FLAGS::C, A >= value);
    SetFlag(FLAGS::Z, A == value);
    SetFlag(FLAGS::N, FLAGS::N & (A - value));
}

void CPU::CPX() {
    uint8_t res, val = value;
    if (isValueRegister) {
        res = X - Read(value);
        val = Read(value);
    }
    else
        res = X - value;
    SetFlag(FLAGS::C, X >= val);
    SetFlag(FLAGS::Z, X == val);
    SetFlag(FLAGS::N, FLAGS::N & res);
}

void CPU::CPY() {
    uint8_t res, val = value;
    if (isValueRegister) {
        res = Y - Read(value);
        val = Read(value);
    }
    else
        res = Y - value;
    SetFlag(FLAGS::C, Y >= val);
    SetFlag(FLAGS::Z, Y == val);
    SetFlag(FLAGS::N, FLAGS::N & res);
}

void CPU::BEQ() {                                       
    if (!(SR & FLAGS::Z)) return;
    clockCycle++;
    uint16_t PCcopy = PC;
    PC += int8_t(value);
    if ((PC & 0xFF00) != (PCcopy & 0xFF00)) clockCycle++;
}

void CPU::BNE() {                                       
    if (SR & FLAGS::Z) return;
    clockCycle++;
    uint16_t PCcopy = PC;
    PC += int8_t(value);
    if ((PC & 0xFF00) != (PCcopy & 0xFF00)) clockCycle++;
}

void CPU::BPL() {                                       
    if (SR & FLAGS::N) return;
    clockCycle++;
    uint16_t PCcopy = PC;
    PC += int8_t(value);
    if ((PC & 0xFF00) != (PCcopy & 0xFF00)) clockCycle++;
}

void CPU::BMI() {                                       
    if (!(SR & FLAGS::N)) return;
    clockCycle++;
    uint16_t PCcopy = PC;
    PC += int8_t(value);
    if ((PC & 0xFF00) != (PCcopy & 0xFF00)) clockCycle++;
}

void CPU::BCC() {                                       
    if (SR & FLAGS::C) return;
    clockCycle++;
    uint16_t PCcopy = PC;
    PC += int8_t(value);
    if ((PC & 0xFF00) != (PCcopy & 0xFF00)) clockCycle++;
}

void CPU::BVC() {                                       
    if (SR & FLAGS::V) return;
    clockCycle++;
    uint16_t PCcopy = PC;
    PC += int8_t(value);
    if ((PC & 0xFF00) != (PCcopy & 0xFF00)) clockCycle++;
}

void CPU::BVS() {
    if (!(SR & FLAGS::V)) return;
    clockCycle++;
    uint16_t PCcopy = PC;
    PC += int8_t(value);
    if ((PC & 0xFF00) != (PCcopy & 0xFF00)) clockCycle++;
}

void CPU::BCS() {
    if (!(SR & FLAGS::C)) return;
    clockCycle++;
    uint16_t PCcopy = PC;
    PC += int8_t(value);
    if ((PC & 0xFF00) != (PCcopy & 0xFF00)) clockCycle++;
}

void CPU::ORA() {
    if (isValueRegister)
        A |= Read(value);
    else
        A |= value;
    SetFlag(FLAGS::Z, A == 0);
    SetFlag(FLAGS::N, FLAGS::N & A);
}

void CPU::EOR() {
    if (isValueRegister)
        A ^= Read(value);
    else
        A ^= value;
    SetFlag(FLAGS::Z, A == 0);
    SetFlag(FLAGS::N, FLAGS::N & A);
}

void CPU::JMP() {
    PC = value;
}

void CPU::BIT() {
    SetFlag(FLAGS::N, Read(value) & FLAGS::N);
    SetFlag(FLAGS::V, Read(value) & FLAGS::V);
    SetFlag(FLAGS::Z, uint8_t(Read(value) & A) == 0);
}

void CPU::INC() {
    Read(value)++;
    SetFlag(FLAGS::Z, Read(value) == 0);
    SetFlag(FLAGS::N, Read(value) & FLAGS::N);
}

void CPU::DEX() {
    X--;
    SetFlag(FLAGS::Z, X == 0);
    SetFlag(FLAGS::N, X & FLAGS::N);
}
void CPU::DEY() {
    Y--;
    SetFlag(FLAGS::Z, Y == 0);
    SetFlag(FLAGS::N, Y & FLAGS::N);
}

void CPU::DEC() {
    Read(value)--;
    SetFlag(FLAGS::Z, Read(value) == 0);
    SetFlag(FLAGS::N, Read(value) & FLAGS::N);
}

void CPU::TXA() {
    A = X;
    SetFlag(FLAGS::Z, A == 0);
    SetFlag(FLAGS::N, A & FLAGS::N);
}

void CPU::PHA() {
    StackPush8b(A);
}

void CPU::PLA() {
    A = StackPop8b();
    SetFlag(FLAGS::Z, A == 0);
    SetFlag(FLAGS::N, A & FLAGS::N);
}

void CPU::TXS() {
    SP = X;
}

void CPU::ROL() {
    uint8_t* ptr = &A;
    if (isValueRegister)
        ptr = &Read(value);              
    bool old = SR & FLAGS::C;
    SetFlag(FLAGS::C, *ptr & FLAGS::N);
    *ptr <<= 1;
    *ptr |= uint8_t(old);
    SetFlag(FLAGS::Z, *ptr == 0);
    SetFlag(FLAGS::N, *ptr & FLAGS::N);
}

void CPU::ROR() {
    uint8_t* ptr = &A;
    if (isValueRegister) 
        ptr = &Read(value);                
    bool old = SR & FLAGS::C;
    SetFlag(FLAGS::C, *ptr & FLAGS::C);
    *ptr >>= 1;
    *ptr |= uint8_t(old) << 7;
    SetFlag(FLAGS::Z, *ptr == 0);
    SetFlag(FLAGS::N, *ptr & FLAGS::N);
}

void CPU::LSR() {
    uint8_t* ptr = &A;
    if (isValueRegister) 
        ptr = &Read(value);                 
    SetFlag(FLAGS::C, *ptr & FLAGS::C);
    *ptr >>= 1;
    SetFlag(FLAGS::Z, *ptr == 0);
    SetFlag(FLAGS::N, *ptr & FLAGS::N);
}

void CPU::ASL() {
    uint8_t* ptr = &A;
    if (isValueRegister) 
        ptr = &Read(value);      
    SetFlag(FLAGS::C, *ptr & FLAGS::N);
    *ptr <<= 1;
    SetFlag(FLAGS::Z, *ptr == 0);
    SetFlag(FLAGS::N, *ptr & FLAGS::N);
}

void CPU::SEC() {
    SetFlag(FLAGS::C, true);
}

void CPU::SEI() {
    SetFlag(FLAGS::I, true);
}

void CPU::CLD() {
    SetFlag(FLAGS::D, false);
}

void CPU::SED() {
    SetFlag(FLAGS::D, true);
}

void CPU::CLV() {
    SetFlag(FLAGS::V, false);
}

void CPU::BRK() {
    StackPush16b(PC);
    SetFlag(FLAGS::B, true);
    StackPush8b(SR);
    SetFlag(FLAGS::B, false);
    PC = (uint16_t(Read(0xFFFE + 1)) << 8) | uint16_t(Read(0xFFFE));
}

void CPU::RTI() {
    SR = StackPop8b();
    SR &= ~FLAGS::B;
    PC = StackPop16b();
}

void CPU::PHP() {
    StackPush8b(SR | FLAGS::B | FLAGS::IGNORED);
    SetFlag(FLAGS::B, false);
    SetFlag(FLAGS::IGNORED, false);
}

void CPU::PLP() {
    SR = StackPop8b();
}

void CPU::NOP() {
    return;
}


