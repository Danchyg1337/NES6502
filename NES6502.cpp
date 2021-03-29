#pragma once
#include <iostream>

#include "NES6502.h"
#include "Defines.h"

bool NES::checkHeader() {
    if (program[0] != 'N' || program[1] != 'E' || program[2] != 'S' || program[3] != 0x1A) return false;
    return true;
}

bool NES::LoadRom(std::string filename, bool rawcode)
{
    std::ifstream file;

    file.open(filename, std::ifstream::binary);
    if (!file.is_open()) {
        std::cout << "No input file " << filename << std::endl;
        return false;
    }
    program = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {});
    if (checkHeader()) {
        PRGnum = program[4];
        CHRnum = program[5];
        flag6 = program[6];
        flag7 = program[7];
        printf("PRG size %i, CHR size %i\nFlag6 %c %c %c %c %c %c %c %c\nFlag7 %c %c %c %c %c %c %c %c\n", PRGnum, CHRnum, BYTE_TO_BINARY(flag6), BYTE_TO_BINARY(flag7));
        size_t PRGsize = 16384 * PRGnum, CHRsize = 8192 * CHRnum;
        CPU6502.Load(program.data() + 16, PRGsize);
        PPU2C02.Load(program.data() + 16 + PRGsize, CHRsize);
    }
    else
        CPU6502.Load(program.data(), program.size());
    CPU6502.ConnectToNes(this);
    PPU2C02.ConnectToNes(this);
    DMAOAM.resize(0x0100);
    return true;
}

void NES::Reset() {
    CPU6502.Reset();
    PPU2C02.Reset();
    clockCycle = 0;
}

void NES::Step() {
    PPU2C02.Step();
    if (!(clockCycle % 3)) CPU6502.Step();
    clockCycle++;
}

void NES::Run() {
    while (true) {
        if (running) {
            //std::this_thread::sleep_for(std::chrono::nanoseconds(delay));
            Step();
        }
    }
}

uint8_t& NES::ReadPPU(uint16_t addr) {
    if (addr >= 0x2000 && addr < 0x2008) {
        return PPU2C02.Read(addr);
    }
    else if (addr >= 0x2008 && addr < 0x4000) {
        //return PPU2C02.GetRegister((addr - 0x2000) % 0x0008);
        return PPU2C02.Read(addr);
    }
}

void NES::WritePPU(uint16_t addr, uint8_t value) {
    //printf("PPUWRITE %04X, %02X\n", addr, value);
    if ((addr >= 0x2000 && addr < 0x2008) || addr == 0x4014) {
        PPU2C02.Write(addr, value);
    }
}

uint8_t& NES::ReadCPU(uint16_t addr) {
    if (addr >= 0x2000 && addr < 0x2008) {
        return CPU6502.Read(addr);
    }
}

void NES::WriteCPU(uint16_t addr, uint8_t value) {
    if (addr >= 0x2000 && addr < 0x2008) {
        CPU6502.Write(addr, value);
    }
}

void NES::SendNMI() {
    CPU6502.NMI();
}



