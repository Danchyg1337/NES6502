#pragma once
#include <vector>

class NES;

class PPU {
	NES* nes = nullptr;
public:
	uint16_t clockCycle = 0;
	int16_t horiLines = 0;

	uint16_t VRAMaddr = 0x0;
	bool addrLatch = false;

	std::vector<uint8_t> CHRROM;
	std::vector<uint8_t> VRAM;
	std::vector<uint8_t> paletteTable;

	uint8_t PPUCTRL;	//2000
	uint8_t PPUMASK;	//2001
	uint8_t PPUSTATUS;	//2002
	uint8_t OAMADDR;	//2003
	uint8_t OAMDATA;	//2004
	uint8_t PPUSCROLL;	//2005
	uint8_t PPUADDR;	//2006
	uint8_t PPUDATA;	//2007
	uint8_t OAMDMA;		//4014

	enum FLAGS : uint8_t {
		B0 = 1,
		B1 = 2,
		B2 = 4,
		B3 = 8,
		B4 = 16,
		B5 = 32,
		B6 = 64,
		B7 = 128
	};

	bool Load(uint8_t* pattern, size_t size);
	void Step();
	void Reset();
	void ConnectToNes(NES* nes);
	uint8_t& Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);
};