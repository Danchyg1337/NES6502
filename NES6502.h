#pragma once
#include <fstream>

#include <map>
#include <vector>
#include <mutex>

#include "CPU.h"
#include "PPU.h"

class NES
{
    uint8_t PRGnum = 1, CHRnum = 0, flag6, flag7;
    bool checkHeader();

public:
    CPU CPU6502;
    PPU PPU2C02;
    std::vector<uint8_t> program;
    bool running = false;

    int delay = 100;
public:

    bool LoadRom(std::string filename, bool rawcode = false);
    void Reset();
    void Step();
    void Run();
    uint8_t& ReadPPU(uint16_t addr);
    void WritePPU(uint16_t addr, uint8_t value);
    uint8_t& ReadCPU(uint16_t addr);
    void WriteCPU(uint16_t addr, uint8_t value);
    void SendNMI();
};

