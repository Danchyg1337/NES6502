#pragma once
#include <fstream>

#include <map>
#include <vector>
#include <mutex>

#include "CPU.h"
#include "PPU.h"
#include "Mapper.h"

class NES
{

    uint8_t PRGnum = 1, CHRnum = 0, console = 0;
    bool mirroring;
    bool battery;
    bool trainer;
    bool hw4screen;
    uint8_t mapperNumber = 0;
    uint8_t checkHeader();

public:
    Mapper* mapper = nullptr;
    CPU CPU6502;
    PPU PPU2C02;
    std::vector<uint8_t> DMAOAM;
    std::vector<uint8_t> cartridge;
    bool running = false;

    int delay = 100;

    uint16_t clockCycle = 0;

    std::unordered_map<uint16_t, std::string> supportedMappers = {
        {0,   "Mapper Zero / No mapper"},
        {2,   "UxROM"},
        {94,  "UxROM"},
        {180, "UxROM"},
    };

public:

    bool LoadRom(std::string filename, bool rawcode = false);
    void Reset();
    void Step();
    void Frame();
    void Run();
    uint8_t& ReadPPU(uint16_t addr);
    void WritePPU(uint16_t addr, uint8_t value);
    uint8_t& ReadCPU(uint16_t addr);
    void WriteCPU(uint16_t addr, uint8_t value);
    void SendNMI();
};

