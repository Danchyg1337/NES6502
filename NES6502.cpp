#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

class CPU {
public:
    CPU(uint8_t* program, size_t size) {
        Reset();
        memory = new uint8_t[2048 * 32];
        memcpy(&memory[PC], program, size);
    }

    uint8_t* memory;

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
        uint8_t command = Fetch();
        if (instructions.find(command) == instructions.end()) InvalidCommand(command);
        else (this->*instructions[command])();
    }

    void InvalidCommand(uint8_t code) {
        printf("Invalid command : %02X\n", code);
    }

    void LDA(){
        uint8_t value = Fetch();
        A = value;
        SR = (SR & ~FLAGS::Z) | (FLAGS::Z * !(bool)A);
        SR = (SR & ~FLAGS::N) | (FLAGS::N & A);
        std::cout<<"LDA\n";
    }

    void TAX() {
        X = A;
        SR = (SR & ~FLAGS::Z) | (FLAGS::Z * !(bool)X);
        SR = (SR & ~FLAGS::N) | (FLAGS::N & X);
        std::cout << "TAX\n";
    }

    void INX() {
        X++;
        SR = (SR & ~FLAGS::Z) | (FLAGS::Z * !(bool)X);
        SR = (SR & ~FLAGS::N) | (FLAGS::N & X);
        std::cout << "INX\n";
    }

    void BRK() {                                        //not completed
        memory[SP] = PC;
        SR = (SR | FLAGS::B);
        std::cout << "BRK\n";
    }

    void ADC() {                                        //not compeled
        uint8_t value = Fetch();
        uint8_t edge = std::min(A, value);
        A = A + value + (SR & FLAGS::C);
        SR = (SR & ~FLAGS::C) | (A < edge);
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
        printf("A %02X\n", this->A);
        printf("X %02X\n", this->X);
        printf("Y %02X\n", this->Y);
        printf("PC %04X\n", this->PC);
        printf("SP %04X\n", this->SP);

    }

private:
    std::unordered_map<uint8_t, void (CPU::*)()> instructions = {
        {0xA9, &CPU::LDA},
        {0xAA, &CPU::TAX},
        {0xE8, &CPU::INX},
        {0x00, &CPU::BRK},
        {0x69, &CPU::ADC}
    };

};

int main()
{
    std::ifstream file;
    file.open("test1.6502", std::ifstream::binary);
    std::vector<uint8_t> saved(std::istreambuf_iterator<char>(file), {});
    uint8_t* program = new uint8_t[saved.size()];
    for (int cnt = 0; cnt < saved.size(); cnt++) program[cnt] = saved[cnt];

    CPU CPU6502(program, saved.size());

    CPU6502.Run();
    saved.clear();
    
}