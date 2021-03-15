#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

class CPU {
public:
    CPU(uint8_t* program, size_t size) {
        Reset();
        memory = new uint8_t[2048 * 32];
        memcpy(&memory[PC], program, size);
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
        std::cout<<"LDA\n";
    }

    void TAX() {
        X = A;
        SetFlag(FLAGS::Z, X == 0);
        SetFlag(FLAGS::N, FLAGS::N & X);
        std::cout << "TAX\n";
    }

    void INX() {
        X++;
        SetFlag(FLAGS::Z, X == 0);
        SetFlag(FLAGS::N, FLAGS::N & X);
        std::cout << "INX\n";
    }

    void BRK() {                                        //not completed
        memory[SP] = PC;
        SetFlag(FLAGS::B, true);
        std::cout << "BRK\n";
    }

    void ADC() {                                        //not compeled
        uint8_t edge = std::min(A, (uint8_t)value);
        A = A + value + (SR & FLAGS::C);
        SetFlag(FLAGS::C, A < edge);
        std::cout << "ADC\n";
    }

    void Show() {
        #define BYTE_TO_BINARY(byte)  \
        (byte & 0x80 ? '1' : '0'), \
        (byte & 0x40 ? '1' : '0'), \
        (byte & 0x20 ? '1' : '0'), \
        (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), \
        (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), \
        (byte & 0x01 ? '1' : '0')

        //system("cls");
        printf("SR %c%c%c%c%c%c%c%c\n", BYTE_TO_BINARY(this->SR));
        printf("PC %04X\n", this->PC);
        printf("SP %04X\n", this->SP);
        printf("A %02X\n", this->A);
        printf("X %02X\n", this->X);
        printf("Y %02X\n", this->Y);

    }

private:
    struct Command {
        void (CPU::* command)();
        void (CPU::* mode)( void (CPU::*)());
        std::string name;
    };

    std::unordered_map<uint8_t, Command> instructions = {
        {0xA9, {&CPU::LDA, &CPU::IMM, "LDA"}},
        {0xAA, {&CPU::TAX, &CPU::IMP, "TAX"}},
        {0xE8, {&CPU::INX, &CPU::IMP, "INX"}},
        {0x00, {&CPU::BRK, &CPU::IMP, "BRK"}},
        {0x69, {&CPU::ADC, &CPU::IMM, "ADC"}}
    };

};



class NES : public olc::PixelGameEngine
{
public:
    NES()
    {
        sAppName = "NES";
    }



public:
    bool OnUserCreate() override
    {
        std::ifstream file;
        file.open("test1.6502", std::ifstream::binary);
        std::vector<uint8_t> program(std::istreambuf_iterator<char>(file), {});
        CPU CPU6502(&program[0], program.size());
        CPU6502.Run();
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        Clear(olc::Pixel(255, 128, 255));
        /*for (int x = 0; x < ScreenWidth(); x++)
            for (int y = 0; y < ScreenHeight(); y++)
                Draw(x, y, olc::Pixel(rand() % 255, rand() % 255, rand() % 255));*/
        DrawString(ScreenWidth() - 50, ScreenHeight() - 10, "HELLO");
        return true;
    }
};


int main()
{
    NES nes;
    if (nes.Construct(256, 240, 4, 4))
        nes.Start();
}