#pragma once
#include <fstream>

#include <deque>
#include <vector>
#include <mutex>

#include "PPU.h"
#include "CPU.h"


class NES
{
    
public:
    CPU CPU6502;

public:

    bool OnUserCreate (std::string filename)
    {
        std::ifstream file;
        
        file.open (filename, std::ifstream::binary);
        if (!file.is_open ()) {
            std::cout << "No input file " << filename << std::endl;
            return false;
        }
        std::vector<uint8_t> program(std::istreambuf_iterator<char>(file), {});
        CPU6502.Load(program.data(), program.size());
        return true;
    }

};

