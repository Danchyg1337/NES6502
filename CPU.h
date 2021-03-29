#pragma once
#include <algorithm>
#include <unordered_map>
#include <vector>

class NES;

#define LOGGER false

class CPU {
    NES* nes = nullptr;
    uint8_t* memory = nullptr;
    uint8_t controllerStatus = 0;

    FILE* log;
public:
    std::vector<uint8_t> PRGROM;

    long long unsigned int totalClock = 0;

    uint16_t clockCycle = 0;
    uint16_t value = 0;
    bool isValueRegister = false;

    uint16_t startAddr = 0x8000;
    uint16_t stackBottom = 0x0100;

    bool controllerStrobe = false;
    int8_t controllerIndex = 7;
    uint8_t controllerValue = 0;

    uint8_t  A = 0;          //Accumulator
    uint8_t  X = 0;          //X Register
    uint8_t  Y = 0;          //Y Register
    uint8_t  SP = 0;         //Stack Pointer
    uint16_t PC = 0;         //Program Counter
    uint8_t  SR = 0;         //Status Register

    enum FLAGS : uint8_t {
        C = 1,              //Carry
        Z = 2,              //Zero
        I = 4,              //Interrupt Disable
        D = 8,              //Decimal Mode
        B = 16,             //Break Command
        IGNORED = 32,
        V = 64,             //Overflow
        N = 128             //Negative
    };

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
        {0xAD, {&CPU::LDA, &CPU::ABS,  "LDA", 3, 4}},
        {0xBD, {&CPU::LDA, &CPU::ABSX, "LDA", 3, 4}},           
        {0xB9, {&CPU::LDA, &CPU::ABSY, "LDA", 3, 4}},           
        {0xA1, {&CPU::LDA, &CPU::INDX, "LDA", 2, 6}},           
        {0xB1, {&CPU::LDA, &CPU::INDY, "LDA", 2, 5}},           
        {0xA2, {&CPU::LDX, &CPU::IMM,  "LDX", 2, 2}},
        {0xA6, {&CPU::LDX, &CPU::ZPG,  "LDX", 2, 3}},
        {0xB6, {&CPU::LDX, &CPU::ZPGY, "LDX", 2, 4}},
        {0xAE, {&CPU::LDX, &CPU::ABS,  "LDX", 3, 4}},
        {0xBE, {&CPU::LDX, &CPU::ABSY, "LDX", 3, 4}},           
        {0xA0, {&CPU::LDY, &CPU::IMM,  "LDY", 2, 2}},
        {0xA4, {&CPU::LDY, &CPU::ZPG,  "LDY", 2, 3}},
        {0xB4, {&CPU::LDY, &CPU::ZPGX, "LDY", 2, 4}},
        {0xAC, {&CPU::LDY, &CPU::ABS,  "LDY", 3, 4}},
        {0xBC, {&CPU::LDY, &CPU::ABSX, "LDY", 3, 4}},           
        {0xAA, {&CPU::TAX, &CPU::IMP,  "TAX", 1, 2}},
        {0xBA, {&CPU::TSX, &CPU::IMP,  "TSX", 1, 2}},
        {0xA8, {&CPU::TAY, &CPU::IMP,  "TAY", 1, 2}},
        {0x98, {&CPU::TYA, &CPU::IMP,  "TYA", 1, 2}},
        {0xE8, {&CPU::INX, &CPU::IMP,  "INX", 1, 2}},
        {0xC8, {&CPU::INY, &CPU::IMP,  "INY", 1, 2}},
        {0x00, {&CPU::BRK, &CPU::IMP,  "BRK", 1, 7}},
        {0x69, {&CPU::ADC, &CPU::IMM,  "ADC", 2, 2}},
        {0x65, {&CPU::ADC, &CPU::ZPG,  "ADC", 2, 3}},
        {0x75, {&CPU::ADC, &CPU::ZPGX, "ADC", 2, 4}},
        {0x6D, {&CPU::ADC, &CPU::ABS,  "ADC", 3, 4}},
        {0x7D, {&CPU::ADC, &CPU::ABSX, "ADC", 3, 4}},
        {0x79, {&CPU::ADC, &CPU::ABSY, "ADC", 3, 4}},
        {0x61, {&CPU::ADC, &CPU::INDX, "ADC", 2, 6}},
        {0x71, {&CPU::ADC, &CPU::INDY, "ADC", 2, 5}},
        {0x29, {&CPU::AND, &CPU::IMM,  "AND", 2, 2}},
        {0x25, {&CPU::AND, &CPU::ZPG,  "AND", 2, 3}},
        {0x35, {&CPU::AND, &CPU::ZPGX, "AND", 2, 4}},
        {0x2D, {&CPU::AND, &CPU::ABS,  "AND", 3, 4}},
        {0x3D, {&CPU::AND, &CPU::ABSX, "AND", 3, 4}},
        {0x39, {&CPU::AND, &CPU::ABSY, "AND", 3, 4}},
        {0x21, {&CPU::AND, &CPU::INDX, "AND", 2, 6}},
        {0x31, {&CPU::AND, &CPU::INDY, "AND", 2, 5}},
        {0x85, {&CPU::STA, &CPU::ZPG,  "STA", 2, 3}},
        {0x95, {&CPU::STA, &CPU::ZPGX, "STA", 2, 4}},
        {0x8D, {&CPU::STA, &CPU::ABS,  "STA", 3, 4}},
        {0x9D, {&CPU::STA, &CPU::ABSX, "STA", 3, 5}},
        {0x99, {&CPU::STA, &CPU::ABSY, "STA", 3, 5}},
        {0x81, {&CPU::STA, &CPU::INDX, "STA", 2, 6}},
        {0x91, {&CPU::STA, &CPU::INDY, "STA", 2, 6}},
        {0x86, {&CPU::STX, &CPU::ZPG,  "STX", 2, 3}},
        {0x96, {&CPU::STX, &CPU::ZPGY, "STX", 2, 4}},
        {0x8E, {&CPU::STX, &CPU::ABS,  "STX", 3, 4}},
        {0x84, {&CPU::STY, &CPU::ZPG,  "STY", 2, 3}},
        {0x94, {&CPU::STY, &CPU::ZPGX, "STY", 2, 4}},
        {0x8C, {&CPU::STY, &CPU::ABS,  "STY", 3, 4}},
        {0x20, {&CPU::JSR, &CPU::ABS,  "JSR", 3, 6}},
        {0x60, {&CPU::RTS, &CPU::IMP,  "RTS", 1, 6}},
        {0x18, {&CPU::CLC, &CPU::IMP,  "CLC", 1, 2}},
        {0xC9, {&CPU::CMP, &CPU::IMM,  "CMP", 2, 2}},
        {0xC5, {&CPU::CMP, &CPU::ZPG,  "CMP", 2, 3}},
        {0xD5, {&CPU::CMP, &CPU::ZPGX, "CMP", 2, 4}},
        {0xCD, {&CPU::CMP, &CPU::ABS,  "CMP", 3, 4}},
        {0xDD, {&CPU::CMP, &CPU::ABSX, "CMP", 3, 4}},
        {0xD9, {&CPU::CMP, &CPU::ABSY, "CMP", 3, 4}},
        {0xC1, {&CPU::CMP, &CPU::INDX, "CMP", 2, 6}},
        {0xD1, {&CPU::CMP, &CPU::INDY, "CMP", 2, 5}},
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
        {0x30, {&CPU::BMI, &CPU::RLT,  "BMI", 2, 2}},           // cycles (+1 if branch succeeds, + 2 if to a new page)
        {0x50, {&CPU::BVC, &CPU::RLT,  "BVC", 2, 2}},           // cycles (+1 if branch succeeds, + 2 if to a new page)
        {0xB0, {&CPU::BCS, &CPU::RLT,  "BCS", 2, 2}},           // cycles (+1 if branch succeeds, + 2 if to a new page)
        {0x70, {&CPU::BVS, &CPU::RLT,  "BVS", 2, 2}},           // cycles (+1 if branch succeeds, + 2 if to a new page)
        {0x09, {&CPU::ORA, &CPU::IMM,  "ORA", 2, 2}},
        {0x05, {&CPU::ORA, &CPU::ZPG,  "ORA", 2, 3}},
        {0x15, {&CPU::ORA, &CPU::ZPGX, "ORA", 2, 4}},
        {0x0D, {&CPU::ORA, &CPU::ABS,  "ORA", 3, 4}},
        {0x1D, {&CPU::ORA, &CPU::ABSX, "ORA", 3, 4}},
        {0x19, {&CPU::ORA, &CPU::ABSY, "ORA", 3, 4}},
        {0x01, {&CPU::ORA, &CPU::INDX, "ORA", 2, 6}},
        {0x11, {&CPU::ORA, &CPU::INDY, "ORA", 2, 5}},
        {0x49, {&CPU::EOR, &CPU::IMM,  "EOR", 2, 2}},
        {0x45, {&CPU::EOR, &CPU::ZPG,  "EOR", 2, 3}},
        {0x55, {&CPU::EOR, &CPU::ZPGX, "EOR", 2, 4}},
        {0x4D, {&CPU::EOR, &CPU::ABS,  "EOR", 3, 4}},
        {0x5D, {&CPU::EOR, &CPU::ABSX, "EOR", 3, 4}},
        {0x59, {&CPU::EOR, &CPU::ABSY, "EOR", 3, 4}},
        {0x41, {&CPU::EOR, &CPU::INDX, "EOR", 2, 6}},
        {0x51, {&CPU::EOR, &CPU::INDY, "EOR", 2, 5}},
        {0x4C, {&CPU::JMP, &CPU::ABS,  "JMP", 3, 3}},
        {0x6C, {&CPU::JMP, &CPU::IND,  "JMP", 3, 5}},
        {0x24, {&CPU::BIT, &CPU::ZPG,  "BIT", 2, 3}},
        {0x2C, {&CPU::BIT, &CPU::ABS,  "BIT", 3, 4}},
        {0xE6, {&CPU::INC, &CPU::ZPG,  "INC", 2, 5}},
        {0xF6, {&CPU::INC, &CPU::ZPGX, "INC", 2, 6}},
        {0xEE, {&CPU::INC, &CPU::ABS,  "INC", 3, 6}},
        {0xFE, {&CPU::INC, &CPU::ABSX, "INC", 3, 7}},
        {0xCA, {&CPU::DEX, &CPU::IMP,  "DEX", 1, 2}},
        {0x88, {&CPU::DEY, &CPU::IMP,  "DEY", 1, 2}},
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
        {0xE9, {&CPU::SBC, &CPU::IMM,  "SBC", 2, 2}},
        {0xE5, {&CPU::SBC, &CPU::ZPG,  "SBC", 2, 3}},
        {0xF5, {&CPU::SBC, &CPU::ZPGX, "SBC", 2, 4}},
        {0xED, {&CPU::SBC, &CPU::ABS,  "SBC", 3, 4}},
        {0xFD, {&CPU::SBC, &CPU::ABSX, "SBC", 3, 4}},
        {0xF9, {&CPU::SBC, &CPU::ABSY, "SBC", 3, 4}},
        {0xE1, {&CPU::SBC, &CPU::INDX, "SBC", 2, 6}},
        {0xF1, {&CPU::SBC, &CPU::INDY, "SBC", 2, 5}},
        {0x78, {&CPU::SEI, &CPU::IMP,  "SEI", 1, 2}},
        {0xD8, {&CPU::CLD, &CPU::IMP,  "CLD", 1, 2}},
        {0x9A, {&CPU::TXS, &CPU::IMP,  "TXS", 1, 2}},
        {0x0A, {&CPU::ASL, &CPU::ACC,  "ASL", 1, 2}},
        {0x06, {&CPU::ASL, &CPU::ZPG,  "ASL", 2, 5}},
        {0x16, {&CPU::ASL, &CPU::ZPGX, "ASL", 2, 6}},
        {0x0E, {&CPU::ASL, &CPU::ABS,  "ASL", 3, 6}},
        {0x1E, {&CPU::ASL, &CPU::ABSX, "ASL", 3, 7}},
        {0x2A, {&CPU::ROL, &CPU::ACC,  "ROL", 1, 2}},
        {0x26, {&CPU::ROL, &CPU::ZPG,  "ROL", 2, 5}},
        {0x36, {&CPU::ROL, &CPU::ZPGX, "ROL", 2, 6}},
        {0x2E, {&CPU::ROL, &CPU::ABS,  "ROL", 3, 6}},
        {0x3E, {&CPU::ROL, &CPU::ABSX, "ROL", 3, 7}},
        {0x6A, {&CPU::ROR, &CPU::ACC,  "ROR", 1, 2}},
        {0x66, {&CPU::ROR, &CPU::ZPG,  "ROR", 2, 5}},
        {0x76, {&CPU::ROR, &CPU::ZPGX, "ROR", 2, 6}},
        {0x6E, {&CPU::ROR, &CPU::ABS,  "ROR", 3, 6}},
        {0x7E, {&CPU::ROR, &CPU::ABSX, "ROR", 3, 7}},
        {0x40, {&CPU::RTI, &CPU::IMP,  "RTI", 1, 6}},
        {0xF8, {&CPU::SED, &CPU::IMP,  "SED", 1, 2}},
        {0xB8, {&CPU::CLV, &CPU::IMP,  "CLV", 1, 2}},
        {0x58, {&CPU::CLI, &CPU::IMP,  "CLI", 1, 2}},
        {0x08, {&CPU::PHP, &CPU::IMP,  "PHP", 1, 3}},
        {0x28, {&CPU::PLP, &CPU::IMP,  "PLP", 1, 4}}
    };

    bool Load(uint8_t* program, size_t size);
    void ConnectToNes(NES* nes);
    void NMI();
    void Reset();
    void IRQ();
    void StackPush16b(uint16_t value);
    void StackPush8b(uint8_t value);
    uint16_t StackPop16b();
    uint8_t StackPop8b();
    uint8_t Fetch();
    uint8_t& Read(uint16_t addr);
    void Write(uint16_t addr, uint8_t value);
    void Run();
    void Step();
    void Call();
    void InvalidCommand(uint8_t code);
    void SetFlag(FLAGS flag, bool set);
    //addressing modes 
    void IMP (void (CPU::* command)());
    void ACC (void (CPU::* command)());
    void IMM (void (CPU::* command)());
    void ZPG (void (CPU::* command)());
    void ZPGX(void (CPU::* command)());
    void ZPGY(void (CPU::* command)());
    void RLT (void (CPU::* command)());
    void ABS (void (CPU::* command)());
    void ABSX(void (CPU::* command)());
    void ABSY(void (CPU::* command)());
    void IND (void (CPU::* command)());
    void INDX(void (CPU::* command)());
    void INDY(void (CPU::* command)());
    //instructions
    void LDA();
    void LDX();
    void LDY();
    void TAX();
    void TSX();
    void TAY();
    void TYA();
    void INX();
    void INY();
    void BRK();
    void ADC();
    void SBC();
    void AND();
    void STA();
    void STX();
    void STY();
    void JSR();
    void RTS();
    void CLC();
    void CMP();
    void CPX();
    void CPY();
    void BEQ();
    void BNE();
    void BPL();
    void BMI();
    void BVS();
    void BCC();
    void BVC();
    void BCS();
    void ORA();
    void EOR();
    void JMP();
    void BIT();
    void INC();
    void DEX();
    void DEY();
    void DEC();
    void TXA();
    void PHA();
    void PLA();
    void TXS();
    void LSR();
    void ASL();
    void SEC();
    void SEI();
    void CLD();
    void NOP();
    void ROL();
    void ROR();
    void RTI();
    void SED();
    void CLV();
    void CLI();
    void PHP();
    void PLP();

};