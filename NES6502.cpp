﻿#include <iostream>
#include <fstream>
#include "Defines.h"
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <unordered_map>
#include <deque>
#include <vector>

class CPU {
public:
    CPU() {}

    void load (uint8_t* program, size_t size) {
        Reset ();
        memory = new uint8_t[2048 * 32];
        memcpy (&memory[PC], program, size);
    }

    uint8_t* memory;

    uint16_t value = 0;
    bool isValueRegister = false;

    uint8_t  A = 0;          //Accumulator
    uint8_t  X = 0;          //X Register
    uint8_t  Y = 0;          //Y Register
    uint16_t SP;             //Stack Pointer
    uint16_t PC;             //Program Counter
    uint8_t  SR;             //Status Register

    enum FLAGS : uint8_t{
        C       = 1,
        Z       = 2,
        I       = 4,
        D       = 8,
        B       = 16,
        IGNORED = 32,
        V       = 64,
        N       = 128
    };

    void Reset() {
        SP = 0x01FF;
        PC = 0x8000;
        SR = FLAGS::IGNORED;
    }


    uint8_t Fetch() {
        uint8_t fetched = memory[PC];
        PC++;
        return fetched;
    }

    void Run() {
        while (!(SR & FLAGS::B)) {
            Call();
            Show();
        }
    }

    void Call() {
        uint8_t opcode = Fetch();
        if (instructions.find(opcode) == instructions.end()) InvalidCommand(opcode);
        else (this->*instructions[opcode].mode)(instructions[opcode].command);
    }

    void InvalidCommand(uint8_t code) {
        printf("Invalid command : %02X\n", code);
    }

    void SetFlag(FLAGS flag, bool set) {
        if (set)
            SR = SR | flag;
        else
            SR = SR & ~flag;
    }

    //addressing modes 
    void IMP(void (CPU::* command)()) {
        (this->*command)();
    }

    void ACC() {
        
    }

    void IMM(void (CPU::*command)()) {
        value = Fetch();
        isValueRegister = false;
        (this->*command)();
    }

    void ZPG(void (CPU::* command)()) {
        value = Fetch();
        isValueRegister = true;
        (this->*command)();
    }

    void ZPGX(void (CPU::* command)()) {
        value = uint8_t(Fetch() + X);
        isValueRegister = true;
        (this->*command)();
    }

    void ZPGY(void (CPU::* command)()) {
        value = uint8_t(Fetch() + Y);
        isValueRegister = true;
        (this->*command)();
    }

    void RLT(void (CPU::* command)()) {
        value = Fetch();
        isValueRegister = false;
        (this->*command)();
    }

    void ABS(void (CPU::* command)()) {
        uint16_t MSB = Fetch();
        uint16_t LSB = Fetch();
        value = (MSB << 8) | LSB;
        isValueRegister = true;
        (this->*command)();
    }

    void ABSX(void (CPU::* command)()) {
        uint16_t MSB = Fetch();
        uint16_t LSB = Fetch();
        value = ((MSB << 8) | LSB) + X;
        isValueRegister = true;
        (this->*command)();
    }

    void ABSY(void (CPU::* command)()) {
        uint16_t MSB = Fetch();
        uint16_t LSB = Fetch();
        value = ((MSB << 8) | LSB) + Y;
        isValueRegister = true;
        (this->*command)();
    }

    void IND(void (CPU::* command)()) {

    }

    void IXIR(void (CPU::* command)()) {

    }

    void IRIX(void (CPU::* command)()) {

    }

    //instructions
    void LDA(){
        if (isValueRegister) A = memory[value];
        else A = value;
        SetFlag(FLAGS::Z, A == 0);
        SetFlag(FLAGS::N, FLAGS::N & A);
    }

    void TAX() {
        X = A;
        SetFlag(FLAGS::Z, X == 0);
        SetFlag(FLAGS::N, FLAGS::N & X);
    }

    void INX() {
        X++;
        SetFlag(FLAGS::Z, X == 0);
        SetFlag(FLAGS::N, FLAGS::N & X);
    }

    void BRK() {                                        //not completed
        memory[SP] = PC;
        SetFlag(FLAGS::B, true);
    }

    void ADC() {                                        //not compeled
        uint8_t edge = std::min(A, (uint8_t)value);
        A = A + value + (SR & FLAGS::C);
        SetFlag(FLAGS::C, A < edge);
    }

    void AND() {
        if (isValueRegister) A &= memory[value];
        else A &= value;
        SetFlag(FLAGS::Z, A == 0);
        SetFlag(FLAGS::N, FLAGS::N & A);
    }

    void STA() {
        memory[value] = A;
    }
    void STX() {
        memory[value] = X;
    }
    void STY() {
        memory[value] = Y;
    }

    void Show() {
        //system("cls");

        printf ("SR %c%c%c%c%c%c%c%c\n", BYTE_TO_BINARY (this->SR));
        printf ("PC %04X\n", this->PC);
        printf ("SP %04X\n", this->SP);
        printf ("A %02X\n", this->A);
        printf ("X %02X\n", this->X);
        printf ("Y %02X\n", this->Y); 

    }

    struct Command {
        void (CPU::* command)();
        void (CPU::* mode)(void (CPU::*)());
        std::string name;
        uint16_t bytes;
        uint8_t cycles;
    };

    std::unordered_map<uint8_t, Command> instructions = {
        {0xA9, {&CPU::LDA, &CPU::IMM,  "LDA", 2, 2}},
        {0xA5, {&CPU::LDA, &CPU::ZPG,  "LDA", 2, 3}},
        {0xB5, {&CPU::LDA, &CPU::ZPGX, "LDA", 2, 4}},
        {0xAA, {&CPU::TAX, &CPU::IMP,  "TAX", 1, 2}},
        {0xE8, {&CPU::INX, &CPU::IMP,  "INX", 1, 2}},
        {0x00, {&CPU::BRK, &CPU::IMP,  "BRK", 1, 7}},
        {0x69, {&CPU::ADC, &CPU::IMM,  "ADC", 2, 2}},
        {0x29, {&CPU::AND, &CPU::IMM,  "AND", 2, 2}},
        {0x85, {&CPU::STA, &CPU::ZPG,  "STA", 2, 3}},
        {0x86, {&CPU::STX, &CPU::ZPG,  "STX", 2, 3}},
        {0x84, {&CPU::STY, &CPU::ZPG,  "STY", 2, 3}}
    };

};


class NES : public olc::PixelGameEngine
{
    CPU CPU6502;

public:
    NES()
    {
        sAppName = "NES";
    }
    

public:
    bool OnUserCreate() override
    {
        std::ifstream file;
        std::string file_name = "test1.6502";
        file.open(file_name, std::ifstream::binary);
        if (!file.is_open()) {
            std::cout << "No input file " << file_name << std::endl;
            return false;
        }
        std::vector<uint8_t> program(std::istreambuf_iterator<char>(file), {});
        CPU6502.load(program.data(), program.size());
        CPU6502.Run();
        return true;
    }

    template<class... Ts> 
    void print_parametr (int x, int y, std::string buff, const char* text, Ts&&... args) {
        sprintf_s (buff.data (), buff.size (), text, args...);
        DrawString (x, y, buff, olc::RED, 1);
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        Clear(olc::Pixel(255, 128, 255));
       
        int nes_width = 256;
        int nes_height = 240;

        for (int x = 0; x < nes_width; x++)
            for (int y = 0; y < nes_height; y++)
                Draw(x, y, olc::Pixel(rand() % 255, rand() % 255, rand() % 255));

        std::string a (50, ' ');
        print_parametr (nes_width + 5, 1, a, "Accamulator %02X", CPU6502.A);
        print_parametr (nes_width + 5, 9, a, "X Register %02X", CPU6502.X);
        print_parametr (nes_width + 5, 17, a, "Y Register %02X", CPU6502.Y);
        print_parametr (nes_width + 5, 25, a, "Stack Pointer %04X", CPU6502.SP);
        print_parametr (nes_width + 5, 33, a, "Program Counter %04X", CPU6502.PC);
        print_parametr (nes_width + 5, 41, a, "Status Register %c%c%c%c%c%c%c%c", BYTE_TO_BINARY(CPU6502.SR));
        int height_ = 41;

        std::deque<std::string> que_;

        uint16_t local_pc = 0x8000;
        uint8_t opcode = CPU6502.memory[local_pc];
       
        while ((local_pc < CPU6502.PC + 5) and opcode != 0x00){ //and !(SR & FLAGS::B))) Break opcode
            std::ostringstream string_stream;
            
            opcode = CPU6502.memory[local_pc];
            auto instruction = CPU6502.instructions[opcode];

            string_stream << instruction.name << " ";

            for (uint16_t i = 0; i < instruction.bytes; i++) {
                string_stream << std::hex << CPU6502.memory[local_pc + i] << " ";
            }
            
            que_.push_back(string_stream.str());
            

            local_pc += instruction.bytes;

            if (que_.size () > 10) que_.pop_front();
        }

        for (int i = 0; i < que_.size(); i++)
            DrawString (nes_width + 5, height_ + 8 * (i + 1), que_[i], olc::RED, 1);
        // FIXME надо пошаговое выполнение
        return true;
    }
};


int main()
{
    NES nes;
    if (nes.Construct(256 + 200, 240, 4, 4))
        nes.Start();
}