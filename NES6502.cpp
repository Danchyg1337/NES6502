#pragma once
#include <iostream>

#include "NES6502.h"
#include "Defines.h"

uint8_t NES::checkHeader() {
    bool isNes = false;
    if (cartridge[0] == 'N' && cartridge[1] == 'E' && cartridge[2] == 'S' && cartridge[3] == 0x1A) isNes = true;
    if (isNes == true && (cartridge[7] & 0x0C) == 0x08) return 2;
    return isNes;
}

bool NES::LoadRom(std::string filename, bool rawcode)
{
    std::ifstream file;

    file.open(filename, std::ifstream::binary);
    if (!file.is_open()) {
        std::cout << "No input file " << filename << std::endl;
        return false;
    }
    cartridge = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {});
    uint8_t ines = checkHeader();
    if (!ines) {
        printf("Not an iNES file\n");
        system("pause");
        return false;
    }
    PRGnum      = cartridge[4];
    CHRnum      = cartridge[5];
    mirroring   = cartridge[6] & 1;
    battery     = cartridge[6] & 2;
    trainer     = cartridge[6] & 4;
    hw4screen   = cartridge[6] & 8;
    mapperNumber = (cartridge[7] & 0xF0) | ((cartridge[6] & 0xF0) >> 4);
    console     = cartridge[7] & 3;
    if (supportedMappers.find(mapperNumber) == supportedMappers.end()) {
        printf("Mapper %i is unsupported\n", mapperNumber);
        system("pause");
        return false;
    }
    
    switch (mapperNumber) {
    case 0:
        mapper = new Mapper;
        break;
    case 2:
        mapper = new UXROM;
    }
    
    printf("iNES v%i\n", ines);
    printf("PRG size %i * 16384, CHR size %i * 8192\nMirroring %i\nTrainer %i\nAdditional RAM %i\nMapper %i\n", PRGnum, CHRnum, mirroring, trainer, battery, mapperNumber);
    size_t PRGsize = 16384 * PRGnum, CHRsize = 8192 * CHRnum;
    PPU2C02.Load(cartridge.data() + 16 + (trainer ? 512 : 0) + PRGsize, CHRsize);
    CPU6502.ConnectToNes(this);
    PPU2C02.ConnectToNes(this);
    mapper->Load(cartridge.data() + 16 + (trainer ? 512 : 0), PRGsize);
    CPU6502.Load();
    DMAOAM.resize(0x0100);
    PPU2C02.mirroringMode = mirroring;
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

void NES::Frame() {
    do {
        Step();
    } while (!PPU2C02.frameIsReady);
}


void NES::Run() {
    if (running) {
        //std::this_thread::sleep_for(std::chrono::nanoseconds(delay));
        Frame();
    }
}

uint8_t& NES::ReadPPU(uint16_t addr) {
    if (addr >= 0x2000 && addr < 0x2008) {
        return PPU2C02.Read(addr);
    }
    else if (addr >= 0x2008 && addr < 0x4000) {
        return PPU2C02.Read(addr);
    }
}

void NES::WritePPU(uint16_t addr, uint8_t value) {
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



