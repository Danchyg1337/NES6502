﻿#pragma once
#include <fstream>

#include <deque>
#include <vector>
#include <mutex>

#include "PPU.h"
#include "CPU.h"


class NES
{
    uint8_t PRGnum = 1, CHRnum = 0, flag6, flag7;
    bool checkHeader() {
        if (program[0] != 'N' || program[1] != 'E' || program[2] != 'S' || program[3] != 0x1A) return false;
        return true;
    }

public:
    CPU CPU6502;
    PPU PPU2C02;
    std::vector<uint8_t> program;
public:

    bool LoadRom(std::string filename, bool rawcode = false)
    {
        std::ifstream file;
        
        file.open (filename, std::ifstream::binary);
        if (!file.is_open ()) {
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
        return true;
    }

    void Step() {

    }

    void Run() {

    }

};

