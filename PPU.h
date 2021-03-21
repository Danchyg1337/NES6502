#pragma once


class PPU {
public:
	uint16_t clockCycle = 0;
	int16_t horiLines = 0;

	uint16_t addr = 0x0;

	std::vector<uint8_t> CHRROM;
	std::vector<uint8_t> VRAM;
	std::vector<uint8_t> paletteTable;

	uint8_t* PPUCTRL;	//2000
	uint8_t* PPUMASK;	//2001
	uint8_t* PPUSTATUS;	//2002
	uint8_t* OAMADDR;	//2003
	uint8_t* OAMDATA;	//2004
	uint8_t* PPUSCROLL;	//2005
	uint8_t* PPUADDR;	//2006
	uint8_t* PPUDATA;	//2007
	uint8_t* OAMDMA;	//4014


	void Link(uint8_t* PPUCTRLaddr, uint8_t* OAMDMAaddr) {
		PPUCTRL = PPUCTRLaddr;
		PPUMASK = PPUCTRLaddr + 1;
		PPUSTATUS = PPUCTRLaddr + 2;
		OAMADDR = PPUCTRLaddr + 3;
		OAMDATA = PPUCTRLaddr + 4;
		PPUSCROLL = PPUCTRLaddr + 5;
		PPUADDR = PPUCTRLaddr + 6;
		PPUDATA = PPUCTRLaddr + 7;
		OAMDMA = OAMDMAaddr;
	}
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

    bool Load(uint8_t* pattern, size_t size) {
		paletteTable.resize(0xFF, 0);
		VRAM.resize(2048, 0);
		CHRROM.resize(size);
		memcpy(CHRROM.data(), pattern, size);
		return true;
    }

	void Step() {
		clockCycle++;
		if (clockCycle > 340) {
			clockCycle = 0;
			horiLines++;
			if (horiLines > 261)
				horiLines = -1;
		}
		if (horiLines == 241 && clockCycle == 1) *PPUSTATUS |= FLAGS::B7;
		if (horiLines == 261 && clockCycle == 1) *PPUSTATUS &= ~FLAGS::B7;
	}

	void Reset() {
		VRAM.clear();
	}
};