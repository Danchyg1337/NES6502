#pragma once

#include "PPU.h"
#include "NES6502.h"

bool PPU::Load(uint8_t* pattern, size_t size) {
	OAM.resize(0x0100, 0);
	paletteRAM.resize(0x20, 0);
	VRAM.resize(0x0800, 0);
	patternTable.resize(0x2000, 0);
	ATtoRender.resize(0x80, 0);
	CHRROM.resize(size);
	memcpy(CHRROM.data(), pattern, size);
	memcpy(patternTable.data(), pattern, 0x2000);
	return true;
}

void PPU::Step() {
	frameIsReady = false;
	if (horiLines == -1 && clockCycle == 1) PPUSTATUS &= ~FLAGS::B7;
	if (horiLines == 0 && clockCycle == 0) clockCycle = 1;
	if (clockCycle > 340) {
		clockCycle = 0;
		horiLines++;
		if (horiLines > 260) {
			horiLines = -1;
			VRAMtoRender = std::vector<uint8_t>(VRAM.begin(), VRAM.begin() + 0x03C0);
			VRAMtoRender.insert(VRAMtoRender.end(), VRAM.begin() + 0x0400, VRAM.begin() + 0x0400 + 0x03C0);
			ATtoRender = std::vector<uint8_t>(VRAM.begin() + 0x03C0, VRAM.begin() + 0x03C0 + 64);
			ATtoRender.insert(ATtoRender.end(), VRAM.begin() + 0x0400 + 0x03C0, VRAM.begin() + 0x0400 +0x03C0 + 64);
			mode8x16	   = PPUCTRL & FLAGS::B5;
			BanktoRenderBG = PPUCTRL & FLAGS::B4;
			BanktoRenderFG = PPUCTRL & FLAGS::B3;
			bgColor = paletteTable[0];
			GetPalette();
			frameIsReady = true;
		}
	}
	if (horiLines == 241 && clockCycle == 1) {
		PPUSTATUS |= FLAGS::B7;
		if (PPUCTRL & FLAGS::B7) nes->SendNMI();
	}



	clockCycle++;
}

void PPU::Reset() {
	VRAM.clear();
	VRAMaddr = 0x0;
	clockCycle = 0;
	horiLines = 0;
}

void PPU::ConnectToNes(NES* nes) {
	this->nes = nes;
}

uint8_t& PPU::Read(uint16_t addr) {
	switch (addr) {
	case 0x2002: {
		data = PPUSTATUS;
		PPUSTATUS &= ~FLAGS::B7;
		addrLatch = false;
		return data;
	}
	case 0x2004:
		return OAMDATA;
	case 0x2007: {
		data = fetched;
		fetched = ReadData(VRAMaddr);
		if (VRAMaddr >= 0x3F00) data = fetched;
		VRAMaddr += (PPUCTRL & FLAGS::B2 ? 32 : 1);
		return data;
	}
	}
}

uint8_t PPU::ReadData(uint16_t addr) {
	if (addr < 0x2000) {
		return patternTable[addr];
	}
	if (addr >= 0x2000 && addr < 0x3000) {
		uint16_t mirrAddr = addr - 0x2000;
		if (mirroringMode == MIRRORING::HORIZONTAL) {
			if		(addr >= 0x2400 && addr < 0x2C00) mirrAddr -= 0x0400;
			else if (addr >= 0x2C00) mirrAddr -= 0x0800;
		}
		if (mirroringMode == MIRRORING::VERTICAL) {
			if (addr >= 0x2800) mirrAddr -= 0x0800;
		}
		return VRAM[mirrAddr];
	}
	if (addr >= 0x3F00 && addr <= 0x3FFF) {
		if (addr % 4 == 0)
			return paletteRAM[0];
		return paletteRAM[(addr - 0x3F00) % 0x20];
	}
}

void PPU::WriteData(uint16_t addr, uint8_t value) {
	if (addr < 0x2000) {
		patternTable[addr] = value;
	}
	if (addr >= 0x2000 && addr < 0x3000) {
		uint16_t mirrAddr = addr - 0x2000;
		if (mirroringMode == MIRRORING::HORIZONTAL) {
			if		(addr >= 0x2400 && addr < 0x2C00) mirrAddr -= 0x0400;
			else if (addr >= 0x2C00) mirrAddr -= 0x0800;
		}
		if (mirroringMode == MIRRORING::VERTICAL) {
			if (addr >= 0x2800) mirrAddr -= 0x0800;
		}
		VRAM[mirrAddr] = value;
	}	
	if (addr >= 0x3F00 && addr <= 0x3FFF) {
		if (addr % 4 == 0)
			paletteRAM[0] = value;
		else
			paletteRAM[(addr - 0x3F00) % 0x20] = value;
	}
}

void PPU::Write(uint16_t addr, uint8_t value) {
	switch (addr) {
	case 0x2000:
		PPUCTRL = value;
		nametableBank = PPUCTRL & 0x03;
		//printf("BANK %i\n", nametableBank);
		baseBankAddr = nametableBank * 0x0400;
		break;
	case 0x2001:
		PPUMASK = value;
		break;
	case 0x2002:
		break;
	case 0x2003:

		break;
	case 0x2004:

		break;
	case 0x2005:
		if (addrLatch) {
			scrollY = value;
			addrLatch = false;
		}
		else {
			scrollX = value;
			addrLatch = true;
		}
		break;
	case 0x2006:
		if (addrLatch) {
			VRAMaddr = value;
			VRAMaddr |= uint16_t(PPUADDR) << 8;
			addrLatch = false;
		}
		else addrLatch = true;
		PPUADDR = value;
		break;
	case 0x2007:
		PPUDATA = value;
		WriteData(VRAMaddr, PPUDATA);
		VRAMaddr += (PPUCTRL & FLAGS::B2 ? 32 : 1);
		break;
	case 0x4014:
		OAM = nes->DMAOAM;
		break;
	}
}

void PPU::GetPalette() {
	bgColor = paletteTable[ReadData(0x3F00) % 64];
	bgPalettes.palettte0 = { paletteTable[ReadData(0x3F00 + 1) % 64],  paletteTable[ReadData(0x3F00 + 2) % 64],  paletteTable[ReadData(0x3F00 + 3) % 64] };
	bgPalettes.palettte1 = { paletteTable[ReadData(0x3F00 + 5) % 64],  paletteTable[ReadData(0x3F00 + 6) % 64],  paletteTable[ReadData(0x3F00 + 7) % 64] };
	bgPalettes.palettte2 = { paletteTable[ReadData(0x3F00 + 9) % 64],  paletteTable[ReadData(0x3F00 + 10) % 64], paletteTable[ReadData(0x3F00 + 11) % 64] };
	bgPalettes.palettte3 = { paletteTable[ReadData(0x3F00 + 13) % 64], paletteTable[ReadData(0x3F00 + 14) % 64], paletteTable[ReadData(0x3F00 + 15) % 64] };

	fgPalettes.palettte0 = { paletteTable[ReadData(0x3F10 + 1) ],  paletteTable[ReadData(0x3F10 + 2) ],  paletteTable[ReadData(0x3F10 + 3) ] };
	fgPalettes.palettte1 = { paletteTable[ReadData(0x3F10 + 5) ],  paletteTable[ReadData(0x3F10 + 6) ],  paletteTable[ReadData(0x3F10 + 7) ] };
	fgPalettes.palettte2 = { paletteTable[ReadData(0x3F10 + 9) ],  paletteTable[ReadData(0x3F10 + 10)], paletteTable[ReadData(0x3F10 + 11) ] };
	fgPalettes.palettte3 = { paletteTable[ReadData(0x3F10 + 13) ], paletteTable[ReadData(0x3F10 + 14) ], paletteTable[ReadData(0x3F10 + 15) ] };
}