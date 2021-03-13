#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

class CPU {
public:
    CPU(uint8_t* memory) {
        this->memory = memory;
        Reset();
    }

    uint8_t* memory;         //Program Memory

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

    uint16_t stack[32];

    void Reset() {
        SP = 0x01FF;
        PC = 0x0;
        SR = FLAGS::IGNORED | FLAGS::B;
    }

    uint8_t Fetch() {
        uint8_t fetched = memory[PC];
        PC++;
        return fetched;
    }

    void Call() {
        uint8_t command = Fetch();
        if (instructions.find(command) == instructions.end()) InvalidCommand();
        else (this->*instructions[command])();
    }

    void InvalidCommand() {
        std::cout << "Invalid command\n";
    }

    void LDA(){
        uint8_t value = Fetch();
        A = value;
        SR = (SR & ~FLAGS::Z) | (FLAGS::Z * !(bool)A);
        SR = (SR & ~FLAGS::N) | (FLAGS::N * (A >> 7));
        std::cout<<"LDA\n";
    }

    void TAX() {
        X = A;
        SR = (SR & ~FLAGS::Z) | (FLAGS::Z * !(bool)X);
        SR = (SR & ~FLAGS::N) | (FLAGS::N * (X >> 7));
        std::cout << "TAX\n";
    }

    void INX() {
        X++;
        SR = (SR & ~FLAGS::Z) | (FLAGS::Z * !(bool)X);
        SR = (SR & ~FLAGS::N) | (FLAGS::N * (X >> 7));
        std::cout << "INX\n";
    }

    void BRK() {
        stack[SP] = PC;
        SR = (SR | FLAGS::B);
        std::cout << "BRK\n";
    }

private:
    std::unordered_map<uint8_t, void (CPU::*)()> instructions = {
        {0xA9, &CPU::LDA},
        {0xAA, &CPU::TAX},
        {0xE8, &CPU::INX},
        {0x00, &CPU::BRK}
    };

};

void ShowCPU(CPU& cpu) {
    #define BYTE_TO_BINARY(byte)  \
        (byte & 0x80 ? '1' : '0'), \
        (byte & 0x40 ? '1' : '0'), \
        (byte & 0x20 ? '1' : '0'), \
        (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), \
        (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), \
        (byte & 0x01 ? '1' : '0')

    printf("SR %c%c%c%c%c%c%c%c\n", BYTE_TO_BINARY(cpu.SR));
}

int main()
{
    std::ifstream file;
    file.open("test1.6502", std::ifstream::binary);
    std::vector<uint8_t> saved(std::istreambuf_iterator<char>(file), {});
    uint8_t* program = new uint8_t[saved.size()];
    for (int cnt = 0; cnt < saved.size(); cnt++) program[cnt] = saved[cnt];

    CPU CPU6502(program);

    for (auto a : saved) CPU6502.Call();
    ShowCPU(CPU6502);
    saved.clear();
    
}