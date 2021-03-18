#pragma once

#include <iostream>
#include <algorithm>
#include <unordered_map>


class CPU {
public:
    int ramsize = 2048 * 32;
    void load (uint8_t* program, size_t size) {
        memory = new uint8_t[ramsize];
        for (int t = 0; t < ramsize; t++) memory[t] = 0;
        Reset ();
        memcpy (&memory[PC], program, size);
    }
    bool running = false;


    uint8_t* memory;

    uint16_t value = 0;
    bool isValueRegister = false;

    uint16_t startAddr = 0x0600;
    uint16_t stackBottom = 0x0100;

    uint8_t  A = 0;          //Accumulator
    uint8_t  X = 0;          //X Register
    uint8_t  Y = 0;          //Y Register
    uint8_t  SP;             //Stack Pointer
    uint16_t PC;             //Program Counter
    uint8_t  SR;             //Status Register

    enum FLAGS : uint8_t {
        C = 1,
        Z = 2,
        I = 4,
        D = 8,
        B = 16,
        IGNORED = 32,
        V = 64,
        N = 128
    };

    void Reset () {
        SP = 0xFF;
        PC = startAddr;
        SR = FLAGS::IGNORED;
        A = 0;
        X = 0;
        Y = 0;
        system ("cls");
    }


    uint8_t Fetch () {
        uint8_t fetched = memory[PC];
        PC++;
        return fetched;
    }

    void Run () {
        while (running) 
            Call();
        
    }

    void Step () {
        Call();
    }

    void Call () {
        //memory[0xfe] = rand() % 256;
        uint8_t opcode = Fetch ();
        if (instructions.find (opcode) == instructions.end ()) InvalidCommand (opcode);
        else (this->*instructions[opcode].mode)(instructions[opcode].command);
    }

    void InvalidCommand (uint8_t code) {
        printf ("Invalid command : %02X\n", code);
    }

    void SetFlag (FLAGS flag, bool set) {
        if (set)
            SR = SR | flag;
        else
            SR = SR & ~flag;
    }

    //addressing modes 
    void IMP (void (CPU::* command)()) {
        (this->*command)();
    }

    void ACC (void (CPU::* command)()) {
        isValueRegister = false;
        (this->*command)();
    }

    void IMM (void (CPU::* command)()) {
        value = Fetch ();
        isValueRegister = false;
        (this->*command)();
    }

    void ZPG (void (CPU::* command)()) {
        value = Fetch ();
        isValueRegister = true;
        (this->*command)();
    }

    void ZPGX (void (CPU::* command)()) {
        value = uint8_t (Fetch () + int8_t (X));
        isValueRegister = true;
        (this->*command)();
    }

    void ZPGY (void (CPU::* command)()) {
        value = uint8_t (Fetch () + int8_t (Y));
        isValueRegister = true;
        (this->*command)();
    }

    void RLT (void (CPU::* command)()) {
        value = Fetch ();
        isValueRegister = false;
        (this->*command)();
    }

    void ABS (void (CPU::* command)()) {
        uint16_t LSB = Fetch ();
        uint16_t MSB = Fetch ();
        value = (MSB << 8) | LSB;
        isValueRegister = true;
        (this->*command)();
    }

    void ABSX (void (CPU::* command)()) {
        uint16_t LSB = Fetch ();
        uint16_t MSB = Fetch ();
        value = ((MSB << 8) | LSB) + int8_t (X);
        isValueRegister = true;
        (this->*command)();
    }

    void ABSY (void (CPU::* command)()) {
        uint16_t LSB = Fetch ();
        uint16_t MSB = Fetch ();
        value = ((MSB << 8) | LSB) + int8_t (Y);
        isValueRegister = true;
        (this->*command)();
    }

    void IND (void (CPU::* command)()) {
        uint16_t LSB = Fetch ();
        uint16_t MSB = Fetch ();
        value = ((MSB << 8) | LSB);
        isValueRegister = true;
        (this->*command)();
    }

    void INDX (void (CPU::* command)()) {
        uint8_t Xval = Fetch ();
        uint8_t pos = Xval + int8_t (X);
        value = memory[pos];
        value |= uint16_t (memory[pos + 1]) << 8;
        isValueRegister = true;
        (this->*command)();
    }

    void INDY (void (CPU::* command)()) {
        uint8_t Yval = Fetch ();
        value = memory[Yval];
        value |= uint16_t (memory[Yval + 1]) << 8;
        value += int8_t (Y);
        isValueRegister = true;
        (this->*command)();
    }

    //instructions
    void LDA () {
        if (isValueRegister) A = memory[value];
        else A = value;
        SetFlag (FLAGS::Z, A == 0);
        SetFlag (FLAGS::N, FLAGS::N & A);
    }

    void LDX () {
        if (isValueRegister) X = memory[value];
        else X = value;
        SetFlag (FLAGS::Z, X == 0);
        SetFlag (FLAGS::N, FLAGS::N & X);
    }

    void LDY () {
        if (isValueRegister) Y = memory[value];
        else Y = value;
        SetFlag (FLAGS::Z, Y == 0);
        SetFlag (FLAGS::N, FLAGS::N & Y);
    }

    void TAX () {
        X = A;
        SetFlag (FLAGS::Z, X == 0);
        SetFlag (FLAGS::N, FLAGS::N & X);
    }

    void INX () {
        X++;
        SetFlag (FLAGS::Z, X == 0);
        SetFlag (FLAGS::N, FLAGS::N & X);
    }

    void INY () {
        Y++;
        SetFlag (FLAGS::Z, Y == 0);
        SetFlag (FLAGS::N, FLAGS::N & Y);
    }

    void BRK () {                                        //not completed
        memory[SP] = PC;
        SetFlag (FLAGS::B, true);
    }

    void ADC () {                                        //not completed
        uint8_t edge = std::min (A, (uint8_t)value);
        A = A + value + (SR & FLAGS::C);
        SetFlag (FLAGS::C, A < edge);
        SetFlag (FLAGS::Z, A == 0);
        SetFlag (FLAGS::V, (~(A ^ value) & (A ^ value)) & FLAGS::N);
        SetFlag (FLAGS::N, FLAGS::N & A);
    }

    void SBC () {                                        //not completed
        uint8_t Acopy = A;
        A = A - value - (1 - (SR & FLAGS::C));
        SetFlag (FLAGS::C, A > Acopy);
        SetFlag (FLAGS::Z, A == 0);
        //SetFlag(FLAGS::V, (~(A ^ value) & (A ^ value)) & FLAGS::N);           //not valid, tbc
        SetFlag (FLAGS::N, FLAGS::N & A);
    }

    void AND () {
        if (isValueRegister) A &= memory[value];
        else A &= value;
        SetFlag (FLAGS::Z, A == 0);
        SetFlag (FLAGS::N, FLAGS::N & A);
    }

    void STA () {
        memory[value] = A;
    }
    void STX () {
        memory[value] = X;
    }
    void STY () {
        memory[value] = Y;
    }

    void JSR () {
        memory[stackBottom + SP] = (PC) >> 8;
        SP--;
        memory[stackBottom + SP] = PC;
        SP--;
        PC = value;
    }

    void RTS () {
        uint16_t toPC;
        SP++;
        toPC = memory[stackBottom + SP];
        memory[stackBottom + SP] = 0x00;
        SP++;
        toPC |= uint16_t (memory[stackBottom + SP]) << 8;
        memory[stackBottom + SP] = 0x00;
        PC = toPC;
    }

    void CLC () {
        SetFlag (FLAGS::C, false);
    }

    void CMP () {
        uint8_t res, val = value;
        if (isValueRegister) {
            res = A - memory[value];
            val = memory[value];
        }
        else
            res = A - value;
        SetFlag (FLAGS::C, A >= val);
        SetFlag (FLAGS::Z, A == val);
        SetFlag (FLAGS::N, FLAGS::N & res);
    }

    void CPX () {
        uint8_t res, val = value;
        if (isValueRegister) {
            res = X - memory[value];
            val = memory[value];
        }
        else
            res = X - value;
        SetFlag (FLAGS::C, X >= val);
        SetFlag (FLAGS::Z, X == val);
        SetFlag (FLAGS::N, FLAGS::N & res);
    }

    void CPY () {
        uint8_t res, val = value;
        if (isValueRegister) {
            res = Y - memory[value];
            val = memory[value];
        }
        else
            res = Y - value;
        SetFlag (FLAGS::C, Y >= val);
        SetFlag (FLAGS::Z, Y == val);
        SetFlag (FLAGS::N, FLAGS::N & res);
    }

    void BEQ () {                                        //not completed
        if (!(SR & FLAGS::Z)) return;
        PC += int8_t (value);
    }

    void BNE () {                                        //not completed
        if (SR & FLAGS::Z) return;
        PC += int8_t (value);
    }

    void BPL () {                                        //not completed
        if (SR & FLAGS::N) return;
        PC += int8_t (value);
    }

    void BCC () {                                        //not completed
        if (SR & FLAGS::C) return;
        PC += int8_t (value);
    }

    void ORA () {
        if (isValueRegister)
            A |= memory[value];
        else
            A |= value;
        SetFlag (FLAGS::Z, A == 0);
        SetFlag (FLAGS::N, FLAGS::N & A);
    }

    void JMP () {
        PC = value;
    }

    void BIT () {
        SetFlag (FLAGS::N, value & FLAGS::N);
        SetFlag (FLAGS::V, value & FLAGS::V);
        SetFlag (FLAGS::Z, value & A);
    }

    void INC () {
        memory[value]++;
        SetFlag (FLAGS::Z, memory[value] == 0);
        SetFlag (FLAGS::N, memory[value] & FLAGS::N);
    }

    void DEX () {
        X--;
        SetFlag (FLAGS::Z, X == 0);
        SetFlag (FLAGS::N, X & FLAGS::N);
    }

    void DEC () {
        memory[value]--;
        SetFlag (FLAGS::Z, memory[value] == 0);
        SetFlag (FLAGS::N, memory[value] & FLAGS::N);
    }

    void TXA () {
        A = X;
        SetFlag (FLAGS::Z, A == 0);
        SetFlag (FLAGS::N, A & FLAGS::N);
    }

    void PHA () {
        memory[stackBottom + SP] = A;
        SP--;
    }

    void PLA () {
        SP++;
        A = memory[stackBottom + SP];
    }

    void LSR () {
        uint8_t* memptr;
        if (isValueRegister)
            memptr = &memory[value];
        else
            memptr = &A;
        SetFlag (FLAGS::C, *memptr & FLAGS::C);
        *memptr >>= 1;
        SetFlag (FLAGS::Z, *memptr == 0);
        SetFlag (FLAGS::N, *memptr & FLAGS::N);
    }

    void SEC () {
        SetFlag (FLAGS::C, true);
    }

    void NOP () {
        return;
    }

    struct Command {
        void (CPU::* command)();
        void (CPU::* mode)(void (CPU::*)());
        std::string name;
        uint8_t bytes;
        uint8_t cycles;
    };

    std::unordered_map<uint8_t, Command> instructions = {
        {0xA9, {&CPU::LDA, &CPU::IMM,  "LDA", 2, 2}},
        {0xA5, {&CPU::LDA, &CPU::ZPG,  "LDA", 2, 3}},
        {0xB5, {&CPU::LDA, &CPU::ZPGX, "LDA", 2, 4}},
        {0xA2, {&CPU::LDX, &CPU::IMM,  "LDX", 2, 2}},
        {0xA6, {&CPU::LDX, &CPU::ZPG,  "LDX", 2, 3}},
        {0xB6, {&CPU::LDX, &CPU::ZPGY, "LDX", 2, 4}},
        {0xAE, {&CPU::LDX, &CPU::ABS,  "LDX", 3, 4}},
        {0xBE, {&CPU::LDX, &CPU::ABSY, "LDX", 3, 4}},           // cycles (+1 if page crossed)
        {0xA0, {&CPU::LDY, &CPU::IMM,  "LDY", 2, 2}},
        {0xA4, {&CPU::LDY, &CPU::ZPG,  "LDY", 2, 3}},
        {0xB0, {&CPU::LDY, &CPU::ZPGX, "LDY", 2, 4}},
        {0xAC, {&CPU::LDY, &CPU::ABS,  "LDY", 3, 4}},
        {0xBC, {&CPU::LDY, &CPU::ABSX, "LDY", 3, 4}},           // cycles (+1 if page crossed)
        {0xAA, {&CPU::TAX, &CPU::IMP,  "TAX", 1, 2}},
        {0xE8, {&CPU::INX, &CPU::IMP,  "INX", 1, 2}},
        {0xC8, {&CPU::INY, &CPU::IMP,  "INY", 1, 2}},
        {0x00, {&CPU::BRK, &CPU::IMP,  "BRK", 1, 7}},
        {0x69, {&CPU::ADC, &CPU::IMM,  "ADC", 2, 2}},
        {0x29, {&CPU::AND, &CPU::IMM,  "AND", 2, 2}},
        {0x85, {&CPU::STA, &CPU::ZPG,  "STA", 2, 3}},
        {0x95, {&CPU::STA, &CPU::ZPGX, "STA", 2, 4}},
        {0x8D, {&CPU::STA, &CPU::ABS,  "STA", 3, 5}},
        {0x9D, {&CPU::STA, &CPU::ABSX, "STA", 3, 5}},
        {0x99, {&CPU::STA, &CPU::ABSY, "STA", 3, 3}},
        {0x91, {&CPU::STA, &CPU::INDY, "STA", 2, 6}},
        {0x81, {&CPU::STA, &CPU::INDX, "STA", 2, 6}},
        {0x86, {&CPU::STX, &CPU::ZPG,  "STX", 2, 3}},
        {0x84, {&CPU::STY, &CPU::ZPG,  "STY", 2, 3}},
        {0x20, {&CPU::JSR, &CPU::ABS,  "JSR", 3, 6}},
        {0x60, {&CPU::RTS, &CPU::IMP,  "RTS", 1, 6}},
        {0x18, {&CPU::CLC, &CPU::IMP,  "CLC", 1, 2}},
        {0xC9, {&CPU::CMP, &CPU::IMM,  "CMP", 2, 2}},
        {0xC5, {&CPU::CMP, &CPU::ZPG,  "CMP", 2, 3}},
        {0xE0, {&CPU::CPX, &CPU::IMM,  "CPX", 2, 2}},
        {0xE4, {&CPU::CPX, &CPU::ZPG,  "CPX", 2, 3}},
        {0xEC, {&CPU::CPX, &CPU::ABS,  "CPX", 2, 4}},
        {0xC0, {&CPU::CPY, &CPU::IMM,  "CPY", 2, 2}},
        {0xC4, {&CPU::CPY, &CPU::ZPG,  "CPY", 2, 3}},
        {0xCC, {&CPU::CPY, &CPU::ABS,  "CPY", 2, 4}},
        {0xF0, {&CPU::BEQ, &CPU::RLT,  "BEQ", 2, 2}},           // cycles (+1 if branch succeeds, + 2 if to a new page)
        {0xD0, {&CPU::BNE, &CPU::RLT,  "BNE", 2, 2}},           // cycles (+1 if branch succeeds, + 2 if to a new page)
        {0x10, {&CPU::BPL, &CPU::RLT,  "BPL", 2, 2}},           // cycles (+1 if branch succeeds, + 2 if to a new page)
        {0x90, {&CPU::BCC, &CPU::RLT,  "BCC", 2, 2}},           // cycles (+1 if branch succeeds, + 2 if to a new page)
        {0x0D, {&CPU::ORA, &CPU::ABS,  "ORA", 3, 4}},
        {0x4C, {&CPU::JMP, &CPU::ABS,  "JMP", 3, 3}},
        {0x6C, {&CPU::JMP, &CPU::IND,  "JMP", 3, 5}},
        {0x24, {&CPU::BIT, &CPU::ZPG,  "BIT", 2, 3}},
        {0xE6, {&CPU::INC, &CPU::ZPG,  "INC", 2, 5}},
        {0xF6, {&CPU::INC, &CPU::ZPGX, "INC", 2, 6}},
        {0xEE, {&CPU::INC, &CPU::ABS,  "INC", 3, 6}},
        {0xFE, {&CPU::INC, &CPU::ABSX, "INC", 3, 7}},
        {0xCA, {&CPU::DEX, &CPU::IMP,  "DEX", 1, 2}},
        {0xC6, {&CPU::DEC, &CPU::ZPG,  "DEC", 2, 5}},
        {0xD6, {&CPU::DEC, &CPU::ZPGX, "DEC", 2, 6}},
        {0xCE, {&CPU::DEC, &CPU::ABS,  "DEC", 3, 6}},
        {0xDE, {&CPU::DEC, &CPU::ABSX, "DEC", 3, 7}},
        {0x8A, {&CPU::TXA, &CPU::IMP,  "TXA", 1, 2}},
        {0x48, {&CPU::PHA, &CPU::IMP,  "PHA", 1, 3}},
        {0x68, {&CPU::PLA, &CPU::IMP,  "PLA", 1, 4}},
        {0xEA, {&CPU::NOP, &CPU::IMP,  "NOP", 1, 2}},
        {0x4A, {&CPU::LSR, &CPU::ACC,  "LSR", 1, 2}},
        {0x46, {&CPU::LSR, &CPU::ZPG,  "LSR", 2, 5}},
        {0x56, {&CPU::LSR, &CPU::ZPGX, "LSR", 2, 6}},
        {0x4E, {&CPU::LSR, &CPU::ABS,  "LSR", 3, 6}},
        {0x5E, {&CPU::LSR, &CPU::ABSX, "LSR", 3, 7}},
        {0x38, {&CPU::SEC, &CPU::IMP,  "SEC", 1, 2}},
        {0xE9, {&CPU::SBC, &CPU::IMM,  "SBC", 2, 2}}
    };

};

